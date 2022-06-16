# scheduler
A stab at co-op multitasking on ARMv7.

<h2>Motive</h2>
<p>A lot of embedded designs want timing guarantees that std::thread can't give.  And I'm too cheap to buy an RTOS or RTOS-capable hardware.  I also wanted to know if this would have solved a previous issue that three separate work-environments had.</p>


<h2>Why not boost::coroutine + boost::fiber?</h2>
<p>I did, actually.  But there was enough uncertainty to ward me off.</p>
<p>Like BOOST_FIBER_NO_ATOMICS had (has?) conflated meanings.</p>
<p>The context switcher didn't save/restore r13 (which has ambiguous meaning).</p>
<p>And I wanted to see if the VFP registers could be omitted in save/restore.</p>
<p></p>
<p>I also just wanted to know if I could. It turns out I can.</p>


<h2>What does it do?</h2>
<p>It runs two greenlets concurrently, until they finish.</p>


<h2>What do you wish it would do?</h2>

* Allow polymorphic greenlets that are picky about what register-sets they save/restore.
* Work with the MMU to forbid access to another greenlet's space, or interrupt on OOB access.
* Schedule greenlet execution based off interrupts
* Handle exception throwing... maybe.  Exceptions are conceptually weird.


<h2>Why not work on this some more?</h2>
<p>Co-routines obsolete the coolest part of this project IMO.  But I couldn't compile the soon-to-be-standard co-routine examples on Godbolt.  And at the same time, this Scheduler was running on a Nucleo board.</p>
<p>I could continue work on this.  But I think I'd rather find a different C++-oriented kernel and work on that.<p>

<h2>What's cool about this?</h2>
<p>I really enjoy allocate_greenlet's API.  It does the pthread thing.  But it also does lambdas, or function-references with variadic arguments.</p>
<p>It also doesn't execute the function until scheduled, which wasn't a guarantee on boost::fiber.</p>
<p>allocate_greenlet will move-construct/copy-construct function arguments into a separate stack, so things like shared_ptr work.</p>
<p>greenlets also take care of function-argument destruction when the function finishes.</p>
<p>There's some inline-assembly, but the compiler does so much of the heavy lifting/logistics/portability boons.</p>
<p>There's an optimization that detects backward-ordered tuples (compared to a similar struct) at compile-time.  The code will reverse the backwards tuples at compile-time.  This removes a lot of unnecessary memory-shuffling on ARM when calling std::apply, in comparison to the naive approach.</p>
<p>SCONS instead of GNU make.  It worked on macOS, and then Windows, and then linux, with a dramatic dearth of fuss.  Bless, SCONS.</p>
