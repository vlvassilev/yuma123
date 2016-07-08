use yangrpc;
use yangcli;
use XML::LibXML;

my $conn = yangrpc::connect("127.0.0.1", 830, "root", "mysecretpass", "hadm1_123","/root/.ssh/id_rsa","/root/.ssh/id_rsa.pub");
defined($conn) || die "Error: yangrpc failed to connect!";

my $names = yangcli::yangcli($conn, "xget /interfaces-state")->findnodes("./data/interfaces-state/interface/name");

for my $name ($names) {
    print $name->toString;
}
print("Done.\n");
