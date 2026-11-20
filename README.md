<p align="center">
    <img height="256px" src="assets/apple/xenios-readme-icon.png" alt="XeniOS app icon" />
</p>

<h1 align="center">NOT XeniOS - Xbox 360 Emulator (the iOS 16+ fork)</h1>

XeniOS is an experimental Apple-focused fork of Xenia, currently based on
[Xenia Edge](https://github.com/has207/xenia-edge). It exists as a fast-moving
place to develop, test, and ship iOS and macOS work while also carrying
platform changes that benefit ARM64 Windows, Linux, and Android. Relevant
improvements are intended to flow back upstream over time.

What "Not XeniOS" is, is a fork of XeniOS that I'm developing on the side to:

1) Help develop and test features that I personally am invested in having in the official release
2) Messing around because I have zero idea what I'm doing and that makes this fun, somehow.

This fork is also especially dedicated to ensuring compatibility with devices on **iOS 16+**, rather than solely iOS 26.
It will hopefully reach feature parity at some point, but merging with the official XeniOS release is ***not*** on the 
board at the moment, as I can't guarantee that merging won't cause unforeseen issues, **as well as reasons I discuss below.** 

Please understand that this particular fork and its branches are NOT the official release, may not maintain parity
with the official release, and at any point in time, could be archived, deprecated, or abandoned,
with no prior warning. If you have any messages or concerns, please find me at my [GitHub](https://github.com/UltraAssassin64).
<p align="center">
  <a>Official XeniOS Links:</a>
  <a href="https://xenios.jp">Website</a> ◦
  <a href="https://github.com/xenios-jp/XeniOS/releases">Releases</a> ◦
  <a href="https://xenios.jp/docs">Docs</a> ◦
  <a href="https://xenios.jp/faq">FAQ</a> ◦
  <a href="https://xenios.jp/compatibility">Compatibility</a> ◦
  <a href="https://discord.gg/QwcTtNKTGf">Discord</a> ◦
  <a href="https://github.com/xenios-jp/XeniOS/issues">Issues</a>
</p>

## Current Focus

- iOS
~~- macOS (Apple Silicon)~~ nope, will work on it later
~~- macOS (Intel)~~ will work on it later

Current published releases focus on iOS and macOS.
For Windows or Linux builds, use [Xenia Edge](https://github.com/has207/xenia-edge) or [Xenia Canary](https://github.com/xenia-canary/xenia-canary).

## Why This Fork Exists

Xenia development is relatively thinly staffed right now, and upstream is not
set up for fast iteration on Apple-specific packaging, documentation, release
flow, and user experience. XeniOS exists so that work can move faster in a
repository where the full product experience can be shaped directly, from the
app itself to releases, docs, compatibility reporting, and the public website
at [xenios.jp](https://xenios.jp).

The goal is not to keep good work siloed here forever. The goal is to iterate
quickly, build a more polished user-facing experience for Apple platforms, and
then contribute the useful technical improvements back upstream into the
broader Xenia community, especially
[Xenia Canary](https://github.com/xenia-canary/xenia-canary).

Again, please note this is </b>NOT</b> the official release of XeniOS. This
particular fork was made to address the gap in compatible versions, as the app
is compiled using Metal 2.3, but relies on the default settings rather than 
forcing it to compile to a minimum of iOS 16. This is fixed in this fork, and
in addition, should support TrollStore JIT in addition to being installable
on iOS 18+ devices. </b></i>iOS 17 support is NOT GUARANTEED.</i></b>

This is no longer an *entirely* accurate statement. While obviously, using
a lower target did allow the emulator to compile, when actually trying to bring
down any of the newer changes, it became an active nightmare to try and run.
The newest version is nowhere near on par with the current "modern" version, but
it does run a version of some of the newer code, patchworked in. The work to re-scope and
bring down ALL of the newer code is in the works at some point, but the key thing is to get
a viable working emulator down first. Some edge cases I've personally experienced
in my favorite games are what I have targeted here, and those changes can
propagate down to other games. Another thing to mention is that the code here
is indeed AI generated, and is probably a spaghetti mess to read through.

As conflicting as it is on my part, this project is not a reflection of my engineering capability. This is a public
personal project that I've decided would be better to just get what I need done. 
I don't intend for this project to be noticed by people in general, but
even in the server, I would prefer not to have people notice this fork as to spare the main fork from questions about "TrollStore when? iOS 16/17 when?". There may
be some benefit to the code that I do generate, but in no way, shape or form do I take pride in this work.
I just made this to satiate an internal need, and have made public because I know that there are others who may want to use this emulator on older hardware.
I understand that I may be blowing things out of proportion in the eyes of some, but for me, I personally find this a huge deal that I'm still on the fence on.


## Downloads

Download XeniOS from
[GitHub Releases](https://github.com/xenios-jp/XeniOS/releases).

- [Latest GitHub release](https://github.com/xenios-jp/XeniOS/releases/latest)
- [All GitHub releases](https://github.com/xenios-jp/XeniOS/releases)


Download NOT XeniOS from my releases [here](https://github.com/UltraAssassin64/XeniOS/releases)!
</i>p.s, you may also want to check the [Actions](https://github.com/UltraAssassin64/XeniOS/actions)
tab and see what the current additions are!</i>

## Quickstart

Start with the public docs at [xenios.jp/docs](https://xenios.jp/docs).

## FAQ

See the public FAQ at [xenios.jp/faq](https://xenios.jp/faq).

## Game Compatibility

Browse currently tracked games on
[xenios.jp/compatibility](https://xenios.jp/compatibility).

To file a compatibility report, use the
[GitHub compatibility tracker](https://github.com/xenios-jp/game-compatibility/issues/new/choose).

## Building

See [building.md](docs/building.md) for setup and information about the
`xb` script. When writing code, check the [style guide](docs/style_guide.md)
and be sure to run clang-format!

## Contributors Wanted!

Have some spare time, know advanced C++, and want to write an emulator?
Contribute! There's a ton of work that needs to be done, a lot of which
is wide open greenfield fun.

**For general rules and guidelines please see [CONTRIBUTING.md](.github/CONTRIBUTING.md).**

Fixes and optimizations are always welcome, especially around Apple platform
performance, UI polish, compatibility coverage, packaging, and tooling.

Start with the
[XeniOS issue tracker](https://github.com/xenios-jp/XeniOS/issues),
join the [XeniOS Discord](https://discord.gg/QwcTtNKTGf), check
[CONTRIBUTING.md](.github/CONTRIBUTING.md), and coordinate before starting
larger work.

## Disclaimer

The goal of this project is to experiment, research, and educate on the topic
of emulation of modern devices and operating systems. **It is not for enabling
illegal activity**. All information is obtained via reverse engineering of
legally purchased devices and games and information made public on the internet
(you'd be surprised what's indexed on Google...).
