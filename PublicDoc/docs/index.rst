.. _welcome:

Welcome to |appname|
====================

.. incimage:: /images/Complex.jpg 500 right

|appname| is a desktop application for creating 3D-printable models. It is
aimed at users who may not have much experience with 3D modeling and should be
easier to learn than other modeling programs.  With it, you can create many
different types of models and export them as STL files that can be sent to a 3D
printer.

|appname| is VR-enabled, meaning that it can be used in VR, but also works in a
standard mouse+keyboard configuration. It can even be used with VR controllers
without the headset on. In that mode, you can use any combination of
controllers, mouse, and keyboard.

|appname| is available for Windows, Mac, and Linux. The Mac version may or not
work in VR due to the `lack of official SteamVR support
<https://store.steampowered.com/news/app/250820/view/2216278054495230717>`_.

|block-image|

.. admonition:: Note

   |appname| is pretty intuitive once you get to know a few basics.  If you
   want to dive right in, you can try the :doc:`Quickstart <Quickstart/index>`.
   Otherwise, take a look at the :doc:`User Guide <UserGuide/index>`
   first. There is also a compact guide to keyboard shortcuts in the
   :doc:`Cheat Sheet <CheatSheet>`.

Installing and Running |appname|
--------------------------------

The application is available to download from `the GitHub release page
<download_site_>`_.

To install it:

 - Windows or Linux: Download the Zip file for your operating system and
   extract the |appname| folder from it. Open the folder and execute the
   |appname| executable file. Note that the application must be run from inside
   this folder.

 - Mac: Download the DMG file and use it to run or install the application.

.. admonition:: VR Only

   Instructions for setting up your VR device for use with |appname| are on
   :doc:`this page <VRSetup>`.

Problems?
---------

If for some reason |appname| crashes, it saves a crash session file in the same
directory it is run in. This file will contain the session up to the point of
the crash, so you can load it to get back to where you were.

Feel free to `open a new issue <https://github.com/pss959/EasyMaker3D/issues>`_
to report a bug or request a feature. Please include the crash file in your
report if applicable.

----

Contents
--------

.. toctree::
   :maxdepth: 2

   self
   VRSetup
   Quickstart/index
   UserGuide/index
   CheatSheet
   ReleaseNotes
