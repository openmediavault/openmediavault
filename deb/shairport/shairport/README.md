ShairPort
=========
By [James Laird](mailto:jhl@mafipulation.org) ([announcement](http://mafipulation.org/blagoblig/2011/04/08#shairport))

What it is
----------
This program emulates an AirPort Express for the purpose of streaming music from iTunes and compatible iPods and iPhones. It implements a server for the Apple RAOP protocol.
ShairPort does not support AirPlay v2 (video and photo streaming).

Build Requirements
------------------
Required:
* OpenSSL

Optionally:
* libao
* PulseAudio
* avahi

Debian/Raspbian users can get the basics with
`apt-get install libssl-dev libavahi-client-dev libasound2-dev`


Runtime Requirements
--------------------
You must be running an mDNS (Bonjour) daemon. On a Mac, this will be running already. Otherwise, you must be running avahi-daemon or Howl.
As an alternative, you may use the tinysvcmdns backend, which embeds a lightweight mDNS daemon. It is, however, way less robust than bonjour or avahi.
Check the [mDNS Backends] section for more information.

How to get started
-------------
```
./configure
make
./shairport -a 'My Shairport Name'
```

The triangle-in-rectangle AirTunes (now AirPlay) logo will appear in the iTunes status bar of any machine on the network, or on iPod/iPhone play controls screen. Choose your access point name to start streaming to the ShairPort instance.

Audio Outputs
-------------
Shairport supports different audio backends.
For a list of available backends and their options, run `shairport -h`.
Note that options are supplied to backends at the end of the commandline, separated by --, for example:
```
shairport -o ao -- -d mydriver -o setting=thing
```

mDNS Backends
-------------
Shairport uses mDNS to advertise the service. Multiple backends are available to perform the task.
For a list of available backends, run `shairport -h`.
The backends prefixed by 'external' rely on external programs that should be present in your path.
By default, shairport will try all backends, in the order they are listed by `shairport -h`, until one works.
You can force the use of a specific backend using `shairport -m tinysvcmdns` for example.

Metadata
--------

The following metadata can be output for the currently playing track:

  * artist
  * title
  * album
  * artwork
  * genre
  * comment

To enable the output of metadata, the `-M <directory name>` flag must be set to
instruct `shairport` where to save the output. This directory must exist. A
fifo named `now_playing` will be created, and records will be written to it
when tracks are changed. The end of a set of metadata is delimited by a
zero-length line. Cover filenames are relative to the cover directory. Files
are not deleted.

An example::

    artist=Arcade Fire
    title=City With No Children
    album=The Suburbs
    artwork=cover-e6450a45ab900815e831434f5ee0499c.jpg
    genre=Rock
    comment=
    

Thanks
------
Big thanks to David Hammerton for releasing an ALAC decoder, which is reproduced here in full.
Thanks to everyone who has worked to reverse engineer the RAOP protocol - after finding the keys, everything else was pretty much trivial.
Thanks also to Apple for obfuscating the private key in the ROM image, using a scheme that made the deobfuscation code itself stand out like a flare.
Thanks to Ten Thousand Free Men and their Families for having a computer and stuff.
Thanks to wtbw.

Contributors to version 1.x
---------------------------
* [James Laird](http://mafipulation.org)
* [Paul Lietar](http://www.lietar.net/~paul)
* [Quentin Smart](http://github.com/sm3rt)
* [Brendan Shanks](http://github.com/mrpippy)
* [Peter Körner](http://mazdermind.de)
* [Muffinman](http://github.com/therealmuffin)
* [Skaman](http://github.com/skaman)
* [Weston](http://github.com/wnielson)
* [allesblinkt](http://github.com/allesblinkt)

Contributors to version 0.x
---------------------------
* [James Laird](mailto:jhl@mafipulation.org), author
* [David Hammerton](http://craz.net/), ALAC decoder
* [Albert Zeyer](http://www.az2000.de), maintainer
* [Preston Marshall](mailto:preston@synergyeoc.com)
* [Mads Mætzke Tandrup](mailto:mads@tandrup.org)
* [Martin Spasov](mailto:mspasov@gmail.com)
* [Oleg Kertanov](mailto:okertanov@gmail.com)
* [Rafał Kwaśny](mailto:mag@entropy.be)
* [Rakuraku Jyo](mailto:jyo.rakuraku@gmail.com)
* [Vincent Gijsen](mailto:vtj.gijsen@gmail.com)
* [lars](mailto:lars@namsral.com)
* [Stuart Shelton](https://blog.stuart.shelton.me/)
* [Andrew Webster](mailto:andywebs@gmail.com)

Known Ports and Tools
---------------------
* Java:
    * [JAirPort](https://github.com/froks/JAirPort)
    * [RPlay](https://github.com/bencall/RPlay)
* Windows:
    * [shairport4w](http://sf.net/projects/shairport4w)
* OS X:
    * [ShairportMenu](https://github.com/rcarlsen/ShairPortMenu), a GUI wrapper as a menu widget
    * [MacShairport](https://github.com/joshaber/MacShairport)
