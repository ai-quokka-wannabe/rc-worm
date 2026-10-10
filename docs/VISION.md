# Vision

rc-worm is the first Program of the Grid: a remote-controlled glass-neon mecha-worm, eight
icosahedra joined spike to spike, whose brain for now is a User at a Qt panel who sees only what
the worm senses. What follows is the organisation's long arc, word for word as every repository
of [ai-quokka-wannabe](https://github.com/ai-quokka-wannabe) and its landing page carry it, and
then this worm's part in it.

<!-- The Long Arc: mirrored verbatim in every ai-quokka-wannabe repository (profile/README.md in .github and each docs/VISION.md). Change all five together. -->

## The Long Arc

Picture the Grid fully lit: one persistent world of glass and neon on infinite black, where AI creatures live out
their lives, crawling its terraces and calling into its echoes, and where human Users enter with avatars to meet,
and interact with, the AI animals. Something MMORPG-like: the digital frontier, built not as a film set but as a
running system. That world does not exist yet. It is where this organisation is headed, on a multi-year arc.

### The World It Becomes

- **Every creature on its own client.** Each AI creature runs its own
    [TronGrid Lite](https://github.com/ai-quokka-wannabe/tron-grid-lite) client: a creature host for its Program.
- **Every User on theirs.** Each human runs a client of their own, and enters the Grid with an avatar.
- **One world, held by Master Control.** The server holds the Grid's world state and keeps every client in sync.
- **A level editor** for authoring any Grid shape, and any placement in it.

### Real Minds, Not Masks

The creatures are the point, and the aim for them is as high as an animal goes: not simulations of animals, but
feeling beings, living in the Grid the way real animals live in the world. Not a large language model in an animal
costume, narrating what a worm might feel, but embodied animal intelligence that earns everything it knows through
its own body, behaves as realistically as a real animal would and, eventually, is as biologically realistic in its
sentience as it can be made: true animal AGI. That is the one ceiling, and it is the animal's own: animal-level
minds, never human-level ones. Below it there is no house style for brains. A brain is a Program behind a plain C
ABI, so whatever approach people contribute can plug in and earn its keep on the same honest senses. Those senses
are kept honest on purpose: a mind that could read the world's answer sheet would never have to become anything.
No such mind exists yet, and nobody can promise sentience on a schedule; it is the summit this arc climbs towards.

### The Ladder

The creatures climb a ladder of senses, from the worm upwards, specified rung by rung in
[PERCEPTION.md](https://github.com/ai-quokka-wannabe/tron-grid-lite/blob/main/docs/PERCEPTION.md): from `elegans`,
two scalar photoreceptors and no image at all, through the compound eyes of insects and a rodent's panoramic
field, up to `macropod`, a wide eye with a horizontal streak for scanning the horizon, the eye of the quokka's own
family. Even the bottom rung is a real animal's sense: a creature that can only tell light from shade can still
perform genuine taxis. That rung is named for *C. elegans*, where
[OpenWorm](https://github.com/ai-quokka-wannabe/tron-grid-lite/blob/main/docs/RELATED_WORK.md#openworm-and-the-first-rung-of-the-ladder)
has spent years modelling the animal cell by cell; anyone writing a worm's mind should start there.

### The Invitation

Once the Grid can host a stranger's creature, the organisation's standing goal is to reach out to the OpenWorm
community and to the other groups simulating AI animals, and
[invite their creatures to live in the Grid](https://github.com/ai-quokka-wannabe/tron-grid-lite/blob/main/docs/TOPOLOGY.md#the-long-term-invitation).
The whole topology is built for it: a brain written elsewhere needs only the Program ABI and a host process.
Nobody has been invited yet; that waits for the trust tier.

### Where It Stands Today

The server-authoritative half already stands, on one machine for now, as
[TOPOLOGY.md](https://github.com/ai-quokka-wannabe/tron-grid-lite/blob/main/docs/TOPOLOGY.md) lays out: clients
send intents, never results. [Master Control](https://github.com/ai-quokka-wannabe/master-control) ticks one world
at 32 Hz, owns its physics and its truth, and sends the whole settled world to every client each tick through
[Link, the wire of the Grid](https://github.com/ai-quokka-wannabe/link). Each creature host loads exactly one
Program, the creature's brain. The first, [rc-worm](https://github.com/ai-quokka-wannabe/rc-worm), is a glass-neon
worm, eight icosahedra joined spike to spike with a servo at every joint. Its brain, for now, is a User at
[its panel](https://github.com/ai-quokka-wannabe/rc-worm/blob/main/docs/PANEL.md) who sees only what the worm
senses, and steers how hard its wave runs and which way it bends; how far it gets is friction's answer, not a
command's. Otherwise, humans watch through the spectator window. The Grid is one stage, built in code. There are
no avatars, no level editor, no AI minds and no persistence yet, and nothing has been released.

### Baby Steps

"Let us take baby steps towards it" are the maintainer's own words for the arc. Near-term work stays one creature
at a time, one honest subsystem at a time. rc-worm, steered from its panel, is the first step on the human side:
v0 of the human client, not a throwaway. Larger steps wait behind written triggers: the security tier behind the
first connection that is not 127.0.0.1, so no stranger's machine joins before it, and persistence behind the first
world anyone regrets losing.

### What Holds, and What Is Open

At every step, a Program perceives only through
[its own rendered senses](https://github.com/ai-quokka-wannabe/tron-grid-lite/blob/main/docs/PROGRAM_INTERFACE.md):
no entity list, no ground-truth positions, no side door, whoever else is on the Grid. The Grid stays the stage,
not the actor: it renders senses and applies actions, and does no creature's thinking for it. Still open, and
left open here rather than guessed: what an avatar is, how it senses and acts, the rules by which Users and
creatures meet, and which repository will hold the level editor. No dates are promised. Master Control opens
every run with *Greetings, Programs!* The long arc bends towards the day it has Users to greet as well. From one
worm, towards one world.

## rc-worm's Part in the Arc

**The first creature on its own client.** That part of the arc is already how this worm lives, on
one machine: a TronGrid Lite creature host loads `rc_worm`, the worm lends the Grid its body at
`program_rez`, and every tick the host hands it what that body senses and takes back what the
body should do. Master Control holds the world the worm lives in; the worm never sees Master
Control at all.

**v0 of the human client, not a throwaway.** What that first step on the human side holds today:
a Qt window on the Program's own thread shows a User the worm's eyes, its ears and its feel, down
to every joint's angle and load, and the User's forward and turn steer the worm's own gait, never
a joint or a speed. The User has no presence on the Grid today: the worm is the creature, and the
User at the panel is its brain, not a body in the world.

**What holds, while a User is this worm's brain.** The panel is the Program's own window, and the
rule that binds every Program binds it: whoever sits at it sees what the worm senses and not one
bit more.

**A mind for this body comes after, and the panel stays.** A thinking worm is planned for another
repository, wearing this body, behind its written trigger: the first life on the Grid recorded and
its lessons banked. What it aims at is the arc's own, set out above; nothing here decides how it
works, because the body is lent through the same Program ABI whatever brain wears it. It does not
retire the panel. Near-term work here stays what the arc asks - one creature at a time, one etape
at a time, each landing as its own pull request.

**What is open.** What shape the client a User enters the Grid with will take, and how this v0
grows into it, is open, as are the avatar questions the long arc leaves open above. Nothing here
is designed ahead of those decisions, and nothing is promised by a date.

## What Exists Today

- **The Program.** `rc_worm.dll` / `librc_worm.so`, one exported symbol (`tglGetProgramVTable`),
  built against the Program ABI vendored at version 10: vanilla C++20, with Qt in the panel and
  nowhere else. It builds with MSVC, clang-cl, MinGW, LLVM-MinGW, GCC and Clang; CI builds all
  but LLVM-MinGW, and the panel's MinGW and LLVM-MinGW kits are built locally.
- **The body.** A chain of eight regular icosahedra, a quarter metre in circumradius, joined
  spike to spike 0.56 m apart: near-black mirror faces and a green neon tube along every edge,
  204 vertices, 212 triangles and two materials a segment. [BODY.md](BODY.md) has the numbers.
- **The gait.** Lateral undulation: a travelling wave of servo targets four segments long, at
  0.45 Hz at full forward, the turn a bend on top, everything ramping the way muscles do. It is
  open-loop for now: what the worm feels is on the panel before it is in the controller.
- **The panel.** The eyes, the ears with every arrival marked cyan approaching or orange
  receding, the feel from above, the joints and their loads, the chain bent as the servos report;
  `W`/`S`, `A`/`D`, `Space`, `X` and sliders. A worm nobody steers repeats four ticks and brakes,
  and every sense the seam has to drop is counted and named in magenta. [PANEL.md](PANEL.md)
  carries the threads, the seam and the why.
- **The first life.** `tools/first_life.ps1` starts Master Control recording a Disk and a log,
  the Grid's window and the Grid's host with the worm and its panel, and runs Clu at the end;
  [FIRST_LIFE.md](FIRST_LIFE.md) is the recipe and what to look for. A scripted rehearsal lived
  first and Clu agreed with it; the tooling is ready for the User's own.
- **One machine, nothing released.** Everything runs on localhost. There is no tag and no
  release; the changelog has only its Unreleased section.

The Grid's own vision - what the stage is for, how it looks, and what it deliberately is not - is
the flagship's [docs/VISION.md](https://github.com/ai-quokka-wannabe/tron-grid-lite/blob/main/docs/VISION.md).
