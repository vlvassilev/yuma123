import yuma
import sys

yuma.init()

print("Loading schema module python-yuma-example.yang")

(res, mod) = yuma.schema_module_load("python-yuma-example.yang")
if(res!=0):
    print("Error: python-yuma-example.yang failed to load!")
    sys.exit(1)

obj = yuma.ncx_find_object(mod,"python-yuma-example")
assert(obj!=None)
val = yuma.val_new_value()
res = yuma.val_set_cplxval_obj(val, obj,"""
{
  "python-yuma-example:python-yuma-example": {
    "message": "hello world"
  }
}
""")

val = yuma.val_new_value()
res = yuma.val_set_cplxval_obj(val, obj,"""
  <python-yuma-example xmlns="http://yuma123.org/ns/python-yuma-example">
      <message>hello world</message>
  </python-yuma-example>
""")

yuma.val_dump_value(val,1)

print("Done.")
