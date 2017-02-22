import paramiko
import socket
import traceback
import os

class netconf:
    def __init__(self):
        self.t = None
        self.chan = None
        self.sock = None
        self.receive_total_data = ""

    def connect(self, arg):
        #arg="server=localhost port=830 user=root password=mysecret123"
        print "connecting: " + arg
        args = arg.split(" ");
        user=os.environ.get('USERNAME')
        password="mysecret123"
        server="localhost"
        port=830
        private_key=os.environ['HOME']+"/.ssh/id_rsa"
        public_key=os.environ['HOME']+"/.ssh/id_rsa.pub"

        for i in range(0,len(args)):
            current_pair = args[i].split("=")
            if current_pair[0] == "user":
                user=current_pair[1]
                #print "user is " + user
            if current_pair[0] == "password":
                password=current_pair[1]
                #print "password is " + password
            if current_pair[0] == "server":
                server=current_pair[1]
                #print "server is " + server
            if current_pair[0] == "port":
                port=int(current_pair[1])
                #print "port is " + str(port)
            if current_pair[0] == "private-key":
                private_key=int(current_pair[1])
            if current_pair[0] == "public-key":
                public_key=int(current_pair[1])

	print "hello" + server
        # now connect
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(30)
            self.sock.connect((server, port))
        except Exception, e:
            print '*** Connect failed: ' + str(e)
            traceback.print_exc()
            return -1

        #self.sock.settimeout(None)
	paramiko.util.log_to_file("filename.log")
        try:
            self.t = paramiko.Transport(self.sock)
            try:
                self.t.start_client()
            except paramiko.SSHException:
                print '*** SSH negotiation failed.'
                return -1
        except Exception, e:
            print '*** Connect failed: ' + str(e)
            traceback.print_exc()
            return -1

        # TODO: check server's host key -- this is important.
        key = self.t.get_remote_server_key()

        self.t.auth_publickey(user, paramiko.RSAKey.from_private_key_file(private_key))

        if not self.t.is_authenticated():
            self.t.auth_password(user, password)

        if not self.t.is_authenticated():
            print '*** Authentication failed. :('
            self.t.close()
            return -1

        self.chan = self.t.open_session()

        self.chan.settimeout(5)
        self.chan.set_name("netconf")
        self.chan.invoke_subsystem("netconf")
        return 0

    def send(self, xml):
        #print "sending: " + xml
        try:
            data = xml + "]]>]]>"
            while data:
                n = self.chan.send(data)
                #print "sent " + str(n)
                if n <= 0:
                    return -1
                data = data[n:]
        except Exception, e:
            print '*** Caught exception: ' + str(e.__class__) + ': ' + str(e)
            traceback.print_exc()
            return -1
	return 0

    def receive(self):
#        print "receiving ..." + self.receive_total_data

        while True:
            xml_len = self.receive_total_data.find("]]>]]>")
            if xml_len >= 0:
                reply_xml = self.receive_total_data[:xml_len]
                self.receive_total_data = self.receive_total_data[xml_len+len("]]>]]>"):]
                break

            try:

                data = self.chan.recv(4096)
            except socket.timeout:
                return (1,[])
            if data:
                #print "got: " + str(data)
                self.receive_total_data = self.receive_total_data + str(data)
            else:
                return (-1,[])


        return (0,reply_xml)

    def rpc(self, xml):
        ret=self.send(xml)
        if(ret!=0):
            return (ret,[])
	(ret,reply_xml)=self.receive()
        return (ret,reply_xml)

    def terminate(self):
        #print "terminating"
        self.chan.close()
        self.t.close()

        return

