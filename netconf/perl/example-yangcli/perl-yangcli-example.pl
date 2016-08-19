use yuma;
use yangrpc;
use yangcli;
use XML::LibXML;

my $conn = yangrpc::connect("127.0.0.1", 830, "root", "mysecretpass","/root/.ssh/id_rsa.pub","/root/.ssh/id_rsa");
defined($conn) || die "Error: yangrpc failed to connect!";

my @names = yangcli::yangcli($conn, "xget /interfaces-state")->findnodes("./rpc-reply/data/interfaces-state/interface/name");

for my $name (@names) {
    print $name->textContent()."\n";
}
print("Done.\n");
