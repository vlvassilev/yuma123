Yuma123
=======

Last Updated: 2015-06-18 (v2.5-1)


What is Yuma123
---------------

The purpose of the Yuma123 project is to provide an opensource YANG API in C
and netconf cli (yangcli) and server (netconfd) appications.

Branching from the last BSD licensed branch of the Yuma project the code has
evolved in the following direction:
- a more mainstream build system based on autoconf/automake was added
- a number of critical bugs have been fixed
- new IETF standards support was added (ietf-nacm, ietf-system, etc.)
- support was added for new YANG extensions


Manual installation steps
-------------------------

~~~
sudo apt-get install git autoconf gcc libtool libxml2-dev libssl-dev libssh2-1-dev

git clone git://git.code.sf.net/p/yuma123/git yuma123-git
cd yuma123-git
autoreconf -i -f
./configure CFLAGS='-g -O0' CXXFLAGS='-g -O0' --prefix=/usr
make
sudo make install
touch /tmp/startup-cfg.xml
/usr/sbin/netconfd --module=helloworld --startup=/tmp/startup-cfg.xml --log-level="debug4" --superuser="$USER"
~~~

If there were no missing dependencies the server is now started with the
example helloworld module.

Tell sshd to listen on port 830 by adding the following 2 lines
to `/etc/ssh/sshd_config`:

~~~
Port 830
Subsystem netconf "/usr/sbin/netconf-subsystem --ncxserver-sockname=830@/tmp/ncxserver.sock"
~~~

And restart the server:
~~~
sudo /etc/init.d/ssh restart
~~~

You can verify everything is OK:

~~~
yangcli --user="$USER" --server=localhost
...
xget /helloworld
~~~

or

~~~
xget /
~~~


Testing with docker
-------------------

This repository has a `Dockerfile` that can be used to create a container that
builds yuma and starts the service. You need a linux with working [docker]
installation to use it.

To build the container:
~~~
docker build -t yuma .
~~~

To start it:
~~~
docker run -it --rm -p 8300:830 -p 2200:22 --name yuma yuma
~~~

The line above maps yuma's netconf port to 8300 on the host. You can connect
to that port with ncclient.

To use *yangcli* as mentioned above, you `docker exec yuma /bin/bash` to enter
the running container. Use *admin* as both the user and password.

