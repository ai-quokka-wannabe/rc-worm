/*
    Copyright (C) 2026 Matej Gomboc https://github.com/ai-quokka-wannabe/rc-worm

    This program is free software: you can redistribute it and/or modify it under the terms of
    the GNU General Public License as published by the Free Software Foundation, either version
    3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
    without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with this program.
    If not, see https://www.gnu.org/licenses/.
*/

/*
    The Qt thread. A QApplication must be built on the thread that runs its event loop, so the
    runner builds it inside the thread's entry function - QThread is the wrong tool here, because
    it is the vanilla side that owns the thread, not Qt. Everything that must happen on the Qt
    thread is posted to an anchor object living there; open and close block until done (they are
    rez and derez, never the tick), stop quits the loop and joins.

    Lost-wakeup rule, kept here as everywhere in this repository: state a waiter's predicate reads
    is written under the same mutex the waiter holds, and the notify happens outside it.
*/

#include <panel/panel.hpp>

#include "window.hpp"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QLibraryInfo>
#include <QtCore/QMetaObject>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>

#include <condition_variable>
#include <cstdlib>
#include <iterator>
#include <mutex>
#include <thread>

#if defined(_WIN32)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace PanelLib
{

    namespace
    {

        struct Runner {
            std::thread thread;
            std::mutex mutex;
            std::condition_variable settled;
            //! True while the QApplication is up and `anchor` may be posted to.
            bool up{false};
            //! True once the thread's entry has decided, up or not, so start() stops waiting.
            bool decided{false};
            QObject* anchor{nullptr};
        };

        Runner& runner()
        {
            static Runner instance;
            return instance;
        }

        /*!
            Whether a window can be drawn at all. On Linux a QGuiApplication without a display
            does not fail - it aborts the process, which inside the Grid is the one thing this
            library must never do - so the question is asked first. Windows always has a desktop
            for the process that runs it.
        */
        bool displayAvailable() noexcept
        {
#if defined(_WIN32)
            return true;
#else
            return (std::getenv("DISPLAY") != nullptr) || (std::getenv("WAYLAND_DISPLAY") != nullptr) || (std::getenv("QT_QPA_PLATFORM") != nullptr);
#endif
        }

        //! The directory this very library was loaded from: where a deployment puts Qt beside it.
        QString thisLibraryDirectory() noexcept
        {
            try {
#if defined(_WIN32)
                HMODULE module{nullptr};
                if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                        reinterpret_cast<LPCWSTR>(&thisLibraryDirectory), &module)
                    == FALSE) {
                    return {};
                }
                wchar_t buffer[4096]{};
                const DWORD length{GetModuleFileNameW(module, buffer, static_cast<DWORD>(std::size(buffer)))};
                if ((length == 0u) || (length >= std::size(buffer))) {
                    return {};
                }
                return QFileInfo{QString::fromWCharArray(buffer, static_cast<int>(length))}.absolutePath();
#else
                Dl_info info{};
                if (dladdr(reinterpret_cast<void*>(&thisLibraryDirectory), &info) == 0 || info.dli_fname == nullptr) {
                    return {};
                }
                return QFileInfo{QString::fromLocal8Bit(info.dli_fname)}.absolutePath();
#endif
            } catch (...) {
                return {};
            }
        }

        /*!
            Whether Qt will find a platform plugin, asked BEFORE a QApplication exists: an
            application that cannot is not refused by Qt, it is aborted - the host process with
            it. Qt looks in the paths QCoreApplication::libraryPaths() names (the kit's own
            prefix, and this library's directory, added here so a deployment beside the worm
            counts), under `platforms/`, for the plugin QT_QPA_PLATFORM names or the desktop's.
        */
        bool platformPluginReachable(const QString& library_directory) noexcept
        {
            try {
                if (!library_directory.isEmpty()) {
                    QCoreApplication::addLibraryPath(library_directory);
                }
                QStringList wanted;
                if (const QString named{qEnvironmentVariable("QT_QPA_PLATFORM")}; !named.isEmpty()) {
                    // "offscreen", "xcb:..." and the like: the plugin's stem is the word before any colon.
                    wanted << named.section(QLatin1Char(':'), 0, 0);
                } else {
#if defined(_WIN32)
                    wanted << QStringLiteral("windows");
#else
                    wanted << QStringLiteral("xcb") << QStringLiteral("wayland");
#endif
                }
                for (const QString& root : QCoreApplication::libraryPaths()) {
                    const QDir platforms{root + QStringLiteral("/platforms")};
                    if (!platforms.exists()) {
                        continue;
                    }
                    for (const QString& stem : wanted) {
                        const QStringList patterns{QStringLiteral("q%1.dll").arg(stem), QStringLiteral("q%1d.dll").arg(stem), QStringLiteral("libq%1.so").arg(stem),
                            QStringLiteral("libq%1*.so").arg(stem)};
                        if (!platforms.entryList(patterns, QDir::Files).isEmpty()) {
                            return true;
                        }
                    }
                }
                return false;
            } catch (...) {
                return false;
            }
        }

        void qtThread(Runner& r)
        {
            // QApplication keeps references to these for its lifetime.
            static int argc{1};
            static char name[]{"rc_worm"};
            static char* argv[]{name, nullptr};
            try {
                QApplication app{argc, argv};
                app.setQuitOnLastWindowClosed(false);
                QObject anchor;
                {
                    const std::lock_guard<std::mutex> lock{r.mutex};
                    r.anchor = &anchor;
                    r.up = true;
                    r.decided = true;
                }
                r.settled.notify_all();
                app.exec();
                {
                    const std::lock_guard<std::mutex> lock{r.mutex};
                    r.anchor = nullptr;
                    r.up = false;
                }
            } catch (...) {
                const std::lock_guard<std::mutex> lock{r.mutex};
                r.anchor = nullptr;
                r.up = false;
                r.decided = true;
            }
            r.settled.notify_all();
        }

    } // namespace

    const char* qtVersion() noexcept
    {
        return qVersion();
    }

    bool start() noexcept
    {
        try {
            Runner& r{runner()};
            std::unique_lock<std::mutex> lock{r.mutex};
            if (r.thread.joinable()) {
                return r.up;
            }
            if (!displayAvailable() || !platformPluginReachable(thisLibraryDirectory())) {
                return false;
            }
            r.up = false;
            r.decided = false;
            r.thread = std::thread{[&r] {
                qtThread(r);
            }};
            r.settled.wait(lock, [&r] {
                return r.decided;
            });
            return r.up;
        } catch (...) {
            return false;
        }
    }

    bool running() noexcept
    {
        Runner& r{runner()};
        const std::lock_guard<std::mutex> lock{r.mutex};
        return r.up;
    }

    Window* open(WormLib::Mailbox& mailbox, const WormLib::BodySnapshot& body) noexcept
    {
        try {
            Runner& r{runner()};
            QObject* anchor{nullptr};
            {
                const std::lock_guard<std::mutex> lock{r.mutex};
                if (!r.up) {
                    return nullptr;
                }
                anchor = r.anchor;
            }
            PanelWindow* window{nullptr};
            QMetaObject::invokeMethod(
                anchor,
                [&mailbox, &body, &window] {
                    window = new PanelWindow{mailbox, body};
                    window->show();
                },
                Qt::BlockingQueuedConnection);
            return reinterpret_cast<Window*>(window);
        } catch (...) {
            return nullptr;
        }
    }

    void close(Window* const window) noexcept
    {
        if (window == nullptr) {
            return;
        }
        try {
            Runner& r{runner()};
            QObject* anchor{nullptr};
            {
                const std::lock_guard<std::mutex> lock{r.mutex};
                anchor = r.up ? r.anchor : nullptr;
            }
            if (anchor == nullptr) {
                return;
            }
            PanelWindow* const panel{reinterpret_cast<PanelWindow*>(window)};
            QMetaObject::invokeMethod(
                anchor,
                [panel] {
                    panel->stopPolling();
                    delete panel;
                },
                Qt::BlockingQueuedConnection);
        } catch (...) {
            // A window that could not be closed is a window on a thread that is gone.
        }
    }

    void stop() noexcept
    {
        try {
            Runner& r{runner()};
            QObject* anchor{nullptr};
            {
                const std::lock_guard<std::mutex> lock{r.mutex};
                if (!r.thread.joinable()) {
                    return;
                }
                anchor = r.up ? r.anchor : nullptr;
            }
            if (anchor != nullptr) {
                QMetaObject::invokeMethod(
                    anchor,
                    [] {
                        QApplication::closeAllWindows();
                        QCoreApplication::quit();
                    },
                    Qt::QueuedConnection);
            }
            r.thread.join();
        } catch (...) {
            // Nothing left to do: the thread is either joined or never ran.
        }
    }

} // namespace PanelLib
