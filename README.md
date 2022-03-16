# webserv
This project is here to make you write your own HTTP server. You will be able to test it with a real browser. HTTP is one of the most used protocol on internet. Knowing its arcane will be useful, even if you won't be working on a website. 

You must write a HTTP server in C++ 98. Program has to take a configuration file as argument, or use a default path. Server must never block and the client can be bounced properly if necessary. It must be non-blocking and use only 1 poll() (or equivalent) for all the I/O operations between the client and the server (listen included). Program should not crash in any circumstances (even when it runs out of memory), and should not quit unexpectedly.
<h2>Final grade : 125/125</h2>
<p>Mandatory part : 100/100</p>
<p>Bonus part : 25/25</p>
<h3>How to use it</h3>
<p>Simply run the setup in main folder :</p>
<pre>
<code>make</code>
</pre>
