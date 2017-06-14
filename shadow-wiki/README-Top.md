Welcome to the wiki!

The process for setting up a development environment is unclear.  This page is a pile of interesting content, and needs refactoring pull requests



==== Open Source Practice

More tasty plants and more happy people!
It's the [network effect](https://en.wikipedia.org/wiki/Network_effect) on
steroids!

The benefits are:
- more people finding this project
- more interest in this project
- more people reading about this project
- more people using this project
- more people talking about this project
- more people contributing to this project, which leads to:
    - higher quality
    - more features
    - fewer flaws
- more tasty plants!
- more happy people eating more tasty plants!

Open source development happens in the open.
That openness has a network effect on steroids.


Search engines are key to this.
People find us through search engines.
If search engines can not find us,
people will not even know we exist.
The search engines need to be able see us.
Turn offs for search engines are:
- registration
    - sites that require registration are hidden to search engines
    and most of the world you want to reach
    - tilt!
    - game over!
    - Do not pass "GO"!
- javascript thwarts many search engines

The complement of open systems are [information
silo](https://en.wikipedia.org/wiki/Information_silo)s.
Information silos are like [roach
motel](https://en.wikipedia.org/wiki/Roach_Motel_%28insect_trap%29)s.
Information checks in, but does not check out.

> ... the cost

Administrivia

- the administration burden to you is insignificant
- delegate to someone
- someone has already volunteered
- ask for someone to take care of the mailing list and it will just happen

----
I strongly
suggest that you use the device in this page :-
https://github.com/ozarchie/hapi/wiki/HAPInode or any ESP32 based device
e.g. (
http://www.ebay.com/itm/ESP-32S-ESP32-Development-Board-2-4GHz-Dual-Mode-Wi
Fi-Bluetooth-Antenna-Module-/201853283621?hash=item2eff64ad25:g:ArkAAOSwax5
Y1OV3)
This device is only USD8 with free shipping . The ESP32 has many advantages
over the ESP8266, which is limited to one analog channel. As we bring more
devices online (pH, RDS, lux) this would be a severe limitation for
development and testing. The ESP32 also has BLE which is ideal for
communicating with low power use devices that can be run on button
batteries and are paired to a specific node. TI has a great SensorTag for
development of such devices. http://www.ti.com/tool/cc2650stk

However, the ESP8266 is very cheap and may be used in single sensor/
control situations.


I will start testing the time changes this weekend.

There are two issues with time as you have identified.
The first is the actual time. This is currently set using ntp and the value
epoch is maintained.
Instead of maintaining epoch, as Tyler has suggested, we could also add a
DS3231 module (<$1) and synchronise that using ntp from the mqtt broker.

The second is the delay() function. As these devices may be used with LiPo
batteries, we want the code to be electron efficient. Arduino has a simple
timer e.g. http://playground.arduino.cc/Code/SimpleTimer which does not use
sleep mode. Other libraries are available that do sue sleep mode e.g.
https://github.com/n0m1/Sleep_n0m1 . However, I have not yet had the time
to verify that they work across mega2560, ESP8266 and ESP32.

Lots of fun testing ahead ..

-------

> Would an Adafruit HUZZAH ESP8266 Breakout board
> <http://www.microcenter.com/product/458078/HUZZAH_ESP8266_Breakout>
> suffice? It is only $10.

Probably, but Amazon stocks an interesting starter kit from Squix
    https://blog.squix.org/
Their starter weather sensor display, which includes an ESP8266, a display, and a DHT-11 sensor, al in a nice storage box
    https://smile.amazon.com/gp/product/B01KE7BA3O/
and I added a protoboard, power conditioning, and wires
    https://smile.amazon.com/gp/product/B016D5LB8U/

That power conditioner asserts it provides both 5 VDC, and 3.3 VDC.  I'll drive it off an old laptop power brick, probably

Recently at their main site, Squix has noted they have made arrangements for outsourced order fulfillment from China for a more capable display with touch-screen, and more]
   https://blog.squix.org/2017/04/esp8266-weather-station-shipping-from-a-fulfillment-center-in-china.html
but there would be longer shipping latencies in getting started 

-----

unanswered question

To be clear, I do not understand the goal of having several network nodes
in IP on lite platforms like ESP for example, and then an application hub
with the Pi-Zero to make the MQTT gateway with the cloud.
Knowing that each node can reach only the MQTT cloud. I think I've read the
basics on the Wiki, but I'm a little lost. I did not find a technical
architecture diagram but only the functional diagram.
Similarly, why the nodes must have RTC/NTP synchronization while the server
cloud record the time series on the Epoch of the server.
Why do nodes "push" on a time synchronization instead of "polling" the
server?

First, the backdrop:

[You aren't gonna need it](https://en.wikipedia.org/wiki/YAGNI) aka YAGNI.

This is a tough one for experienced programmers,
especially myself, who say:

> "But I **know** that I **am** going to need it.".

YAGNI is an exaggeration for emphasis. The plain way of saying it is.

> When you need some code, go ahead and write it, 
> **but not until you actually need it**.

Prediction is difficult, especially of the future.
Often the future turns out differently than anticipated. That is key. 
Adding stuff for the anticipated future affects other code.
So the code that was added or changed for what was anticipated, 
turns out to be either unnecessary or has to be changed. 
The effect cascades through code.
Code that is never used or never used as written, 
wastes time writing it, and wastes time reading it,
and introduces little (and sometimes not so little) 
complications elsewhere to accommodate it.
The indirect unnecessary complications are particularly pernicious.

The YAGNI approach avoids the problems of wasted time and unnecessary
complexity. 
YAGNI does this by not coding stuff until actually needed, 
even when (or **especially when) one knows** it will be needed in the
future.

> As I commented previously, the control of lamps is quite
> different to control of pumps.
I wrote commit 173f039, while remembering your earlier comment 
about lamps and pumps being different.
So when lamps and pumps are handled differently,
treat them differently, but not until then.
The only differences between controlPumps() and controlLamps() in the 
[current master branch](https://github.com/mayaculpa/hapi/tree/2e7d15d0) of
[hapi_controls.ino](https://github.com/mayaculpa/hapi/blob/2e7d15d/src/dumb
_module/arduino/hapinode/hapinode.ino) are:
- function name
- comments
- whitespace

So I refactored the common code to a separate function.
It is an small incremental improvement.

When the needs changes, we'll change the code, but not until then.

> We need to maintain separation of these.

That's why I left the controlPumps() and controlLamps() separate
instead of combining into only function. 
Avoiding that consolidation now might be a bit of YAGNI impurity on my
part.

> My suggestion before was that if we went to a common piece of
> code, that a definition of the parameters of that code was set
> (e.g. turn-on time, duration, turn-off time etc)

There might be some nuance in your use of the word "set".
I am interpreting your use of it to mean that 
the definition of the parameters will never (or almost never) change.

In the past, it was a high virtue for me to anticipate the future needs of
code.
So in this example, I would not consolidate the functions into one
until I absolutely, positively knew that they would never
change as far as the parameters are concerned.

This is exactly what YAGNI is about. 
YAGNI blew my mind the first time I heard about it.
I thought it was nonsense, violating my judgment
about what the overall quickest easiest way to get the finished 
program done, not some short-sighted optimization for the present.
I would gladly do more work now to avoid much work later.
YAGNI tells me that such judgment is wrong more often that it is right;
that doing more anticipatory work now just creates even more work later,
and unnecessarily so. 

I am slow to change, but I am changing.
I am warming up slowly to YAGNI, hence commit 173f039.
When the needs become different, let's change the code, but not until then.
For now, the common part is refactored out.

Thanks to Jason Green (@gJigsaw) for talking about this at the last Python
meeting.

---

I wonder about comparing YAGNI with lean manufacturing that has
minimal inventory instead of large inventory in anticipation of future
needs.

---

By the way,
[rebasing](https://git-scm.com/book/en/v2/Git-Branching-Rebasing) 
is powerful, fun, dangerous, and very useful. @PedroSFreitas has used it.

Unix is a tool builder's culture.
Think of Lego bricks.
Explore how the following bricks are combined.

    grep -r '  *$' *
    grep -rl '  *$' *
    vi -o `grep -rl '  *$' *`  ;# <Esc>:qa to get out of vi
    find * -name '*.py' -print
    find * -name '*.py' -exec grep '  *$' {} \;
    find * -name '*.py' -exec grep -l '  *$' {} \;
    find * -name '*.py' -exec grep -l '  *$' {} \; | while read f; do
t=`mktemp`; echo sed -e 's/  *$//' "$f" "$t";echo mv -f "$t" "$f";done
    find * -name '*.py' -exec grep -l '  *$' {} \; | while read f; do
t=`mktemp`; sed -e 's/  *$//' <"$f" >"$t";mv -f "$t" "$f";done

> Unix is a tool builder's culture.

Not sure, but I believe I read a similar _definition_ of Unix on K&R. Or
maybe on another book.

unlikely:  so far as I know 'tool builder's culture' is original to @herrold

This makes explicit what we should be doing
and that we have already been doing in varying degrees.

1. Make it easy for the person who handles your pull request to merge your
stuff.
In particular, a pull request should not have any merge conflicts.

2. Make it easy for yourself to resolve merge conflicts.
   Let's call the branch that you want to have your branch merged into, 
   the target branch.
   For example mayaculpa's dev branch is a common target branch.
   1. Whenever the target branch changes,
   merge that target branch with your branch. The sooner one does this, 
   the smaller and easier the merge conflicts are to resolve.
   The more your branch and the target branch diverge,
  the smaller and easier the merge conflicts are to resolve.
   The more your branch and the target branch diverge,
   the harder it will be for you to resolve the merge conflicts.
   Make it easier on yourself by merging early and often.
   2. On one's own branch, it is better to have many small commits
   than one huge commit. When a merge with the target branch
   has many conflicts, one could abandon that particular merge,
   then attempt multiple smaller merges between the target branch
   and your older commits. 
   That can break up a big merge with many complicated conflicts
   into several smaller merges, each with less complicated conflicts.
   Also, neverminding merges, having many small commits 
   makes it easier to go back and figure out how something got screwed up
   when the inevitable bugs find you.
   3. When you want to issue a pull request,
   make sure that your branch is a child of the target branch.
   That makes the merge a trivial fast-forward merge.
   The person handling your pull request will love you for it.
   If your branch is not already a child of the target branch,
  The person handling your pull request will love you for it.
   If your branch is not already a child of the target branch,
   merge the target branch into your branch.
   That makes your branch a child of the target branch.
   4. Sometimes, your pull request has no merge conflicts 
   when you create it, but has merge conflicts later. 
   This can happen when someone else's pull request is accepted before
yours.
   Their changes to the target branch can create merge conflicts for your
pull request.
   In that case merge the target branch with your branch again.
   Then your branch is a child of the target branch again,
   so it will not have merge conflicts and will be easy to merge.

Obviously, this is reinventing the wheel.
What web pages codify this stuff and are easy for beginners to understand?

> The code compiles and is currently running ...

Thanks for testing it.

Merging with master would create a new version of software 
that has not been tested, so you should change your pull request
to merge into mayaculpa's dev branch, where one expects to 
retest it.

-------

There are ESP8266 emulators in software, as well as ones for the Arduino class hardware

There are cross-compilers, some running under QEMU, others in Docker Containers

There are CI tools in profusion to validate a proposed commit will work, before it is merged

------------
The most interesting changes are in 65ea8bf and 751ef4e
where you close the database or zeroconf in the finally clauses.

What would happen in the finally clause 
if sqlite3.connect() or Zeroconf() provoked an exception,
so database or zeroconf was undefined?

I wonder about context handlers versus try/except/finally.
What can we learn from Raymond Hettinger's presentation
[Best practices for beautiful intelligible
code](http://pyvideo.org/pycon-us-2015/beyond-pep-8-best-practices-for-beau
tiful-inte.html)?

---------


What instructions do we need? Step-by-step for setting up a node? 


I am still confused -- the documentation says 'arduino', but and ESP / Node device is depicted. I have a drawer full of each sitting around, but what 'cookbook' steps:

    to take hardware and wire it up (shown as to a Node ESP class unit in the Fritzing diagram, which I can match, and
    set up a needed compilation environment (is the Arduino tarball used, or one of the 'in the cloud' ESP tools?)
    then to pull the sources, and
    then to compile it, and
    then to 'flash' it over to the Arduino or ESP, and
    then how to architect a testing set-up, and within that,
    -- configure it to be either a end spoke, or
    -- a central control node?

and only then, see if it does something useful

There is a lot of 'embedded' or assumed knowledge, which needs to be reduced to steps which may be printed, and 'checked off'; as done, and to know that one will end up with, say, at least a gateway control node:

    reporting IoT data from time to time up to a MQTT visualization site,
    pulling configuration and time data, and distributing it 'downstream' to end-point spoke client nodes
    compiling and sending 'upstream' that downstream client node data

There is talk in the last prior comment of a code review. What is the pull request number, so it might be reviewed?

-----------

@TylerReedMC The wiki can't be publicly edited. Perhaps it would be worth
looking at moving the wiki contents into the readme folder within the repo
so that we can perform pull requests and include documentation updates with
corresponding code changes? It is an unfortunate limitation that pull
requests can't be opened on the Github wiki.

.... this remains to be seen -- PR's welcomed breaking this massive page down to bite size chunks

-----

What is 'done done' -- how do we know what remains to be done


We will have succeeded when a non-technical person that we do not know is
able
to successfully use one of our arduino based things, just by following our
instructions on github, without any coaching or hand-holding by us. 

We need instructions to install, compile, download, and run the Arduino
software. That would include:

- requirements
    - hardware
        - development host
        - arduino
            - say which boards are supported
                - be specific
            - what other hardware is required
           - what other hardware is required
                - power supply
                - cables
            - what hardware configuration is required
                - for example, cables or I/O modules
    - software
        - development host (computer that runs Arduino IDE)
            - how to download and install development system
            (ok to refer to specific Arduino URL for this)
            - how to install git
            - how to install hapi software (with git)
                - in correct directory
            - how to install libraries needed by our project
            (I sure hope this can be automated).
            - how to compile
               - might refer to Arduino dox with specific URL
                - augment with our own prose to cover what Arduino's dox do
not
                  cover.
        - on arduino hardware
            - how to download compiled software to it
            (might be able to refer to Arduino dox for some of this
            augmented by our own prose and/or programs)

The above is just a rough outline.

It would be good to have one complete example based on a 
[minimum viable
product](https://en.wikipedia.org/wiki/Minimum_viable_product).

Some things, such as downloading and installing the Arduino IDE,
can be done well by referring to a specific URL.
Some things have to be done manually,
and should be explained in prose, 
sometimes augmented with pictures.
Things that can be automated, should be.
For example the prose of 
[INSTALL.rst](https://github.com/mayaculpa/hapi/blob/17048b3c8d80d90ee13dc8
10e32acfd606e51841/src/smart_module/INSTALL.rst)
has a command
./[INSTALL.sh](https://github.com/mayaculpa/hapi/blob/17048b3c8d80d90ee13dc
810e32acfd606e51841/src/smart_module/INSTALL.sh),
to execute by manually typing (or copying and pasting).
The executed command, automates parts of the installation.

The docs do not need to be fancy.
Plain text dox that are comprehensive and accurate suffice.
We should be able to generate other formats, such as HTML, PDFs, and man
pages,
from the source code for the documentation.
[Sphinx](https://en.wikipedia.org/wiki/Sphinx_%28documentation_generator%29
)
does this well with 
[reStructuredText](https://en.wikipedia.org/wiki/ReStructuredText).
Documentation source code in reStructuredText is readable as is 
and the other formats can be generated from the doc source code.

---

By the way, this discussion started on pull request #106 
and was moved here because the issue of documentation 
is so much more than just the little code change in pull request #106.

----


> It is an unfortunate limitation that pull requests can't be opened on the
Github wiki.

So commit the documentation source code in the same repo
as the regular (i.e., non-documentation) source code.

Also, consider generating the wiki automatically from documentation source
code.
Documentation source code for the wiki might be in a separate directory.
Sphinx groks that very well.

> Step-by-step for setting up a node?

Yes, very much so.

We will have succeeded when a non-technical person that we do not know 
is able to successfully use our stuff, 
just by following our instructions on github, 
without any coaching or hand-holding by us.

> I'd like to merge Archie's Wiki with upstream, 
> but maybe @TylerReedMC would like to take a look first.

and later he wrote:

> But a review is necessary

That is what the dev branch and pull requests are for.
[Release early, release
often](https://en.wikipedia.org/wiki/Release_early,_release_often).
Just throw the wiki into some directory (perhaps "wiki"),
add it to the repo and issue a pull request for it.
Maybe someone else will convert whatever form the wiki is in
to reStructuredText that Sphinx can automatically generates
web sites, PDFs, and more from.

