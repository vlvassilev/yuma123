import sys

if sys.version_info < (3,10):
    from distutils.core import setup,Extension
else:
    from setuptools import setup,Extension

yuma = Extension('yuma',
                   sources = ['yuma.c'],
                   extra_compile_args=['-I/usr/include/yuma/platform', '-I/usr/include/yuma/ncx' , '-I/usr/include/yuma/agt'], extra_link_args=['-lyumancx', '-lyumaagt'])
yangrpc = Extension('yangrpc',
                   sources = ['yangrpc.c'],
                   extra_compile_args=['-I/usr/include/yuma/platform', '-I/usr/include/yuma/ncx' , '-I/usr/include/yuma/yangrpc'], extra_link_args=['-lyumancx', '-lyangrpc'])

setup(name='yangcli',
	  version='0.0.1',
	  description="Python yangcli",
	  author="Vladimir Vassilev",
	  author_email="vladimir@lightside-instruments.com",
	  url="http://yuma123.org/wiki",
	  packages=["yangcli"],
	  license="Apache License 2.0",
	  platforms=["Posix; OS X; Windows"],
	  #classifiers=[]
	  #scripts=['scripts/myscript']
          ext_modules=[yuma, yangrpc],
	  )
