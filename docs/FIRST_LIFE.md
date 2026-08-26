# The first life

A worm whose every action a human chose is the creature a world is debugged with. This page is
the recipe for living one, what to look for while you do, and where what you find goes.

## What runs

Three processes, three console windows, all on one machine:

| Process | Command | Its job |
|---|---|---|
| Master Control | `master-control <port> --disk <out>/life.disk --log <out>/life.log` | the world, recorded to a Disk and every intent logged |
| The Grid's window | `TronGridLite 127.0.0.1:<port> --window` | the User's eyes: fly with WASD, look with the mouse, Tab captures the cursor |
| The Grid's host | `TronGridLite 127.0.0.1:<port> --program rc_worm` | rezzes the worm from `programs/` and opens the panel from inside itself |

`tools/first_life.ps1` starts all three, waits for Master Control to end (Ctrl+C in its window,
or close it: the world stops on request - the tick in hand finishes, the log gets its end
line, the Disk closes - and the host then leaves on its own), and runs Clu on what was recorded:

```powershell
.\tools\first_life.ps1 -Grid <path>\TronGridLite.exe -MasterControl <path>\master-control.exe
```

The Grid's `programs/` directory must hold the deployed worm - the DLL and the Qt beside it,
judged a closed set (PANEL.md § Deployment):

```powershell
cmake --install build\windows-msvc --config Release --prefix build\windows-msvc-deploy
python tools\check_deploy.py build\windows-msvc-deploy\programs
Copy-Item -Recurse -Force build\windows-msvc-deploy\programs\* <grid>\programs\
```

No Qt on the PATH is needed: the Grid loads a Program with its own directory on the search
path, and the panel puts that directory on Qt's.

## At the keys

The panel is the worm's senses and its three controls, nothing more (PANEL.md § The window):

- `W`/`S` drive forward and back at the body's bound (1 m/s for the first body), `A`/`D` turn
  left and right (a right angle a second), `Space` calls once, `X` brakes. The sliders hold a
  course when no key is held.
- Release the keys and the worm brakes within four ticks: the panel's silence rule. Close the
  panel's process and it brakes the same way.
- The eyes are two discs, head and tail, one sample each: a value above one is the eye looking
  at a neon tube. The ears light the early bins with the Grid's own hum; a call of your own
  arrives on the next tick, loudest and first, at both ears - marked cyan (approaching) or
  orange (receding) at its onset, and remembered for a second in the line under the histogram
  with the tick it came in (0.58 ms while standing: the ear is 0.2 m from the voice). The feel
  shows every contact with the floor, and what a riser feels like when you drive into one.

## What the rehearsal found

Before the User's life, a scripted one: keys posted to the panel drove the worm seven metres in
a weave with two calls, the window watching, Clu agreeing (1600 ticks, 50 hashes). What it
showed:

- The worm hears its own call as the ABI promises: three arrivals at each ear, the direct path
  at 0.58 ms and two echoes near 12 ms, loudest and first.
- Master Control could not be asked to stop. Closing its window or Ctrl+C killed it, the Disk
  and the log ended where the last tick left them, and Clu said so every time ("the log has
  no end line - the world did not stop on request"). The recording was whole - every tick is
  written as it happens - but a life should end with an end line. Filed as master-control
  issue #31 and fixed there the same day: Ctrl+C (or closing the window) is now a request,
  the tick in hand finishes, the log ends, the Disk closes with its farewell, and Clu has
  nothing to say about the ending. A second Ctrl+C ends the process at once, the old way.
- Nothing yet in the Grid's picture disagreed with the panel's feel. That is what your life is
  for.

## What to look for

The purpose is what the life shows wrong. Watch for, and write down with the tick number from
the panel's status line:

- **The body and the world disagree.** The window shows the worm somewhere the panel's contacts
  do not explain; a riser the eyes saw that the feel never touched; a slip the feel reports on a
  floor that looks flat.
- **Sound out of place.** A call heard before it was made, an arrival at an ear with no source in
  the window, an echo from a wall that is not there, a hum louder at one ear than the other
  while standing still.
- **Silence that is not yours.** The status line going magenta ("the Grid has been silent")
  while the world's window keeps moving: a tick the host missed, on the record as a refusal in
  Master Control's log.
- **The window versus the Disk.** Anything the window drew that a later `--replay` of the Disk
  draws differently.

## Where it goes

Clu's verdict ends the life: `master-control clu life.log life.disk` re-simulates every logged
intent and compares its hashes with the Disk's. Agreed means the Disk is the world it
describes; anything else names the tick that lies.

What the life showed wrong goes back as issues to the repository whose business it is - the
Grid (rendering, senses, hosting), Master Control (physics, the roster, the record) or Link (the
wire) - each with the tick, the Disk and the log attached. `docs/BODY.md` and `docs/PANEL.md`
are this repository's own; anything about the worm's body or panel is an issue here.

The Disk and the log of the first life are kept: they are the trigger for the thinking worm
(TODO.md § Later, with triggers), which will be asked to live the same life.
