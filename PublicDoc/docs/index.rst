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

   |appname| is pretty intuitive once you get to know a few basics. You can try
   out the :doc:`Tutorials <Tutorials/index>` or take a look at the :doc:`User
   Guide <UserGuide/index>` first. There is also a compact guide to keyboard
   shortcuts in the :doc:`Cheat Sheet <CheatSheet>`.

Installing and Running |appname|
--------------------------------

  - *Windows or Linux*:

    - Open the `the GitHub release page <download_site_>`_ in a browser.
    - Download the Zip file for your operating system.
    - Extract all files wherever you like; this will create a folder called
      |appname|.
    - Double-click on the |appname| executable in this folder to run the
      application.

  - *Mac*:

    - Open the `the GitHub release page <download_site_>`_ in a browser.
    - Download the DMG file.
    - Double-click or install the DMG file to run the |appname| executable.

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
   Tutorials/index
   UserGuide/index
   CheatSheet
   ReleaseNotes
