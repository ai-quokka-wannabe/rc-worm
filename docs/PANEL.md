# The panel

The User is the brain. A Qt Widgets window on the Program's own thread shows what the worm senses
and takes the three controls the ABI carries; the tick never waits for it. This page carries the
threads, the seam, the silence rule, the tests and the deployment - and the why of each.

## The threads, and the order they stop

| Thread | Started by | Owns | Talks through | Stops when |
|---|---|---|---|---|
| **The Grid's tick thread** | the host | `program_tick`, `program_rez`, `program_derez`, the worm | the mailbox: publishes a `SensesSnapshot`, takes an `Intent` | the Grid's business |
| **The Qt thread** | `library_init` (`PanelLib::start`), one `std::thread` | the one `QApplication`, an anchor `QObject`, every window | the mailbox: takes a `SensesSnapshot`, offers an `Intent`; nothing else | `library_shutdown` (`PanelLib::stop`): quit, then join |

Three rules the table enforces:

- **`program_tick` never blocks and never throws.** It copies what the Grid lent into the worm's
  own snapshot, publishes it under a mutex held for the copy, takes the newest intent under the
  same, and returns. It never posts to the Qt thread, never waits for a paint, never allocates.
- **Nothing Qt crosses the seam.** `src/panel/include/panel/panel.hpp` is plain C++20; the
  window reads `WormLib::SensesSnapshot` and `WormLib::BodySnapshot` and writes
  `WormLib::Intent`, all plain structs. Only `src/panel/` includes a Qt header (CLAUDE.md § Rules).
- **A window never outlives its worm.** `program_rez` opens one (blocking until it exists on the
  Qt thread - rez is not the tick), `program_derez` closes it to completion before the worm is
  deleted, `library_shutdown` joins the thread before it returns so the host may unload.

Shutdown, in order: every window closed at its derez; `stop()` posts `closeAllWindows` +
`quit` to the anchor and joins; the `QApplication` dies with the thread's entry function. The
`QApplication` is built **inside** the thread's entry function because Qt requires the event
loop's thread to be the one that built it; `QThread` is the wrong tool here, since the vanilla
side owns the thread.

The lost-wakeup rule, as everywhere in the organisation's C++: state a waiter's predicate reads
(`Runner::up`, `Runner::decided`) is written under the same mutex the waiter holds, and the
notify happens outside it.

A headless Linux is not an error. `PanelLib::start` refuses when no `DISPLAY`, `WAYLAND_DISPLAY`
or `QT_QPA_PLATFORM` is set, because a `QGuiApplication` without a display does not fail - it
aborts the process, which inside the Grid is the one thing this library must never do. The worm
then stands unsteered, which is a worm still.

The same refusal guards the platform plugin. Qt looks for `platforms/qwindows.dll` in its
library paths - the kit's own prefix and the host **executable's** directory, never the DLL's -
so a Qt deployed beside `rc_worm.dll` in `programs/` is invisible to it until the panel says
otherwise: before any `QApplication` exists, `start` asks which directory this very library was
loaded from (`GetModuleHandleEx` from an address of its own; `dladdr` on Linux), adds it with
`QCoreApplication::addLibraryPath`, and then looks for the plugin `QT_QPA_PLATFORM` names or the
desktop's under `platforms/` in every library path. Found: the thread starts. Not found: the
panel refuses, the worm stands - because Qt's own answer to a missing platform plugin is to
abort the host. Both were found the hard way on the first live run: the Grid's process died
with "could not find the Qt platform plugin", and the fix is the pre-check, not a dismissal.

One more thing a harness learns: Windows applies a launcher's "start hidden" flag
(`STARTUPINFO.wShowWindow`) to the **first window the process shows**, and inside the Grid that
is the panel. A host launched normally shows it; a test harness launching the host hidden hides
the panel with it. Not a bug of the panel's, but worth knowing before chasing one.

## The seam

`src/worm/seam.hpp`, vanilla C++20. One `Mailbox` per creature, one slot each way:

- **Senses, tick to panel.** `publishSenses` copies a `SensesSnapshot` (some thirty kilobytes:
  every eye's samples, every ear's band-by-bin energy and its arrivals, every contact, the
  vestibular numbers) and stamps a generation. `takeSenses(out, seen)` copies only what is newer
  than `seen`. Latest wins: a panel that polls slower than the Grid ticks sees the newest tick,
  never a backlog.
- **Intent, panel to tick.** `offerIntent` is latest-wins for forward speed and turn rate; a
  **call is latched** - the loudest vocalisation offered since the tick last took is what the
  tick gets, and then it is cleared. A call is one burst per tick (PROGRAM_INTERFACE.md), the
  panel polls at 25 ms and the Grid ticks at 31.25 ms, so without the latch the panel's own next
  poll could overwrite a call before any tick heard it. With it a call sounds exactly once.
- **Fixed capacities, counted drops.** The snapshot never allocates. A body richer than
  `SEAM_EYES_MAX` eyes of `SEAM_EYE_SAMPLES_MAX` samples, `SEAM_EARS_MAX` ears of
  `SEAM_EAR_BANDS_MAX` x `SEAM_EAR_BINS_MAX`, or `SEAM_CONTACTS_MAX` contacts is not silently
  trimmed: every drop is counted in the snapshot and the panel says so in magenta, by name.
  No information may be lost without a word.

## The silence rule

A tick that finds no new intent repeats the last one for `PANEL_REPEAT_TICKS` (four: an eighth of
a second at 32 Hz) and then brakes - zeroes, and the body stops. The panel offers an intent on
**every poll**, changed or not, so a live panel is a stream and a stalled or dead one is silence
the worm can tell within five ticks. A repeat repeats the motion, never the voice. The wire's own
rule (`LNK_ACTIONS_REPEAT_TICKS`) has the same shape one layer down; this one is the panel's.

`Worm::lastApplied()` names how each tick's answer came to be - `Fresh`, `Repeated`, `Braked` -
for the tests and for a log.

## The window

Drawn from the seam's structs at the Qt side's edge, nothing more than the worm knows:

- **Eyes**: each eye's samples on an equirectangular map of the body frame (forward at the
  centre, up at the top), a disc per sample where it looks, sized by its acceptance angle,
  filled with its value through the same tone curve the Grid's window uses (one channel grey,
  three as they come). The first body's two eyes of one sample each are two discs.
- **Ears**: the band-by-bin histogram each ear is, bands as rows with their edges in kHz, bins as
  columns; each arrival a column at its onset, cyan approaching, orange receding. An arrival is
  one tick's event, gone before a human reads it, so each ear keeps its last arrivals for a
  second, dimmed, and says the tick they came in, how many, the first's onset and radial
  velocity. The worm's own call is the proof: 0.58 ms at both ears (0.2 m at the speed of
  sound) and two echoes off the floor and a riser, seen in the first rehearsal. Since the
  chain scrapes (master-control #34) and the Grid delivers scrapes to creature ears
  (tron-grid-lite #119), a dragging worm hears its own spikes on the floor: both ears light in
  the earliest bins with fading tails - the head's scratch and the seven segments at their
  spacings behind it - and the arrival count stays zero, because a scrape has no onset to
  range from. Energy without an arrival is a scrape; a column is a call.
- **Feel**: the contacts where they happened on the body from above, each with its normal, and
  the numbers - impulse, depth, slip - beside; the proprioceptive speeds, the specific force, the
  angular velocity, the irradiance. A text row that shares its height with the plan box stops
  at the box's edge with an ellipsis rather than writing through it. Along the bottom runs the
  declared chain, side on: the segments the body's rez lends the Grid, icosahedra joined spike
  to spike over a floor line, drawn in the worm's neon because side on an icosahedron's visible
  outline is its neon, the head brightest with a dot of ink for its eye end. It is the
  declaration and nothing more - it never moves, because where the chain is and how it waves is
  the world's, heard through the ears, never echoed back to the panel.
- **Controls**: forward, turn (left positive, as the ABI has it) and voice sliders scaled to the
  body's own bounds; `W`/`S`, `A`/`D` override the sliders while held, `Space` calls once at the
  voice slider's strength, `X` brakes. The status line says which tick it drew, how many intents
  it offered, and turns magenta when the Grid has been silent for a second.

The stylesheet is scoped to the window by its object name, never set on the application: the
host process owns whatever else Qt draws. The window opens at what its views ask for, bounded
by the screen; the header states the declaration - the chain included - and
`PanelWindow::headerText()` says it for a test.

## The tests

- `seam_tests` (deviceless, the flagship's `testing`): copies whole with drops counted; the
  mailbox's latest-once and the latch; the worm's fresh / repeated x4 / braked sequence with the
  test playing the panel; two thousand offers from another thread never lose the newest word.
- `tst_panel` (Qt Test, on Qt Test's own thread): the window draws the newest tick, offers what
  the keys say, latches a call once; two thousand publishes from another thread while it draws,
  the slowest under a generous bound with the measured value in the failure message. Shapes
  adopted from the owner's `claude-chats-browser`; a `QTRY_` re-evaluates once after it is
  satisfied, so a take that consumes is made to stick.
- **Under ThreadSanitizer, with the panel on** (CI, "TSan with Qt"): the same two suites
  with every access instrumented, on Qt's offscreen platform with the glib dispatcher off,
  so no xcb, X11, D-Bus or glib is in the sanitised process - uninstrumented, built without
  frame pointers, their mutex use confuses the sanitiser's model beyond any suppression's
  reach (the first run said so, in twenty-six reports). Qt itself is not instrumented
  either, so the hand-offs across its blocking queued invokes are made visible by an
  acquire/release atomic on either side in `panel.cpp` - an annotation of what Qt's
  semaphore already guarantees, not a fix - and `tools/tsan.supp` names Qt's own libraries
  and nothing else. The seam is a `std::mutex` the sanitiser sees whole, and a race there is
  the real thing this leg exists to catch. The one tight timing bound is widened under the
  sanitiser, with the measured value still in the message.
- **clang-tidy's concurrency checks, as errors** (CI, on the Linux Clang build with the panel
  on): the `.clang-tidy` the editor runs names every family; CI runs `concurrency-*` alone
  over the sources - the calls that are not thread-safe, `getenv` and `localtime` and their
  kin, which a seam between two threads must never make - with Qt's generated code left out.
- `vtable_tests` (through the DLL, as the Grid loads it): with the panel built, `library_init`
  starts the Qt thread and each rez opens a real window; two hundred ticks of two steered-by-
  nobody worms are zeroes and quick; derez closes, shutdown joins. Under `xvfb-run` in CI.

## Deployment

The Grid looks in `programs/`; the worm and what it needs land there:

- `cmake --install <build> --prefix <where>` installs `rc_worm.dll` / `librc_worm.so` to
  `<where>/programs`. On Windows with the panel built, `windeployqt --compiler-runtime` is run
  on the DLL there, without translations, software OpenGL, the D3D compiler and the plugin
  families a panel of labels and sliders never loads.
- `tools/check_deploy.py <where>/programs` judges the result: starting from `rc_worm.dll` and
  every plugin under it, every import must be beside the worm or Windows' own (the Universal C
  and Visual C++ runtimes count as Windows' own - the redistributable is what
  `--compiler-runtime` ships - as does Windows' ICU, which the MSVC kits link rather than ship).
  The closed set today is six libraries: the worm, Qt Core, Gui and Widgets, the platform plugin
  and the style. CI runs it on every Windows Qt leg.
- The Grid finds them because it loads a Program with its own directory on the search path
  (tron-grid-lite `LoadLibraryEx` with `LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR`; `$ORIGIN` on Linux) -
  proven on the desk with no Qt on the PATH.
- On Linux the library's RPATH is `$ORIGIN`, so a Qt placed beside it is found first and the
  system's otherwise; bundling a Linux Qt is not done here - the Grid on Linux has a kit or a
  distribution Qt, and `ldd` is recorded per Qt build for the day that changes.

## Not yet

- One window per creature; the Grid may rez several, and each gets its own.
- No settings persistence, no shortcuts beyond the six keys: the ABI carries three numbers and
  the panel shows no more than the worm knows.
