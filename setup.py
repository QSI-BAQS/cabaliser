import distutils.command.build
from setuptools import setup

class BuildCommand(distutils.command.build.build):
    def run(self):
        distutils.command.build.build.run(self)
        # Run makefile here
           

setup(
    name='cabaliser',
    packages=['cabaliser'],
    package_dir={
        '':'src'},
    include_package_data=True,
)
