#!/usr/bin/env python3

from setuptools import setup

setup(
    name='pss_outline',
    version='1.0.0',
    author='Paul S. Strauss',
    author_email='pss@acm.org',
    description='Python-Markdown extension to wrap logical sections',
    py_modules=['pss_outline'],
    install_requires=['Markdown>=2.0',],
    classifiers=[
        'Development Status :: 4 - Beta',
        'Operating System :: OS Independent',
        'License :: OSI Approved :: BSD License',
        'Intended Audience :: Developers',
        'Environment :: Web Environment',
        'Programming Language :: Python',
        'Topic :: Text Processing :: Filters',
        'Topic :: Text Processing :: Markup :: HTML'
    ]
)
