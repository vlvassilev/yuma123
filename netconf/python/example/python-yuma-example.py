import yuma
import sys

yuma.init()

print("Loading schema module python-yuma-example.yang")

(res, mod) = yuma.schema_module_load("python-yuma-example.yang")
if(res!=0):
    print("Error: python-yuma-example.yang failed to load!")
    sys.exit(1)

(res, root_val) = yuma.cfg_load("python-yuma-example.xml")
if(res!=0):
    print("Error: python-yuma-example.xml failed to load!")
    sys.exit(1)

(res, python_yuma_example_val) = yuma.val_find_child(root_val, "python-yuma-example", "python-yuma-example")
if(res!=0):
    print("Error: Missing /python-yuma-example container!")
    sys.exit(1)

(res, message_val) = yuma.val_find_child(python_yuma_example_val, "python-yuma-example", "message")
if(res!=0):
    print("Error: Missing /python-yuma-example/message leaf!")
    sys.exit(1)

print(yuma.val_string(message_val))

print("Done.")
