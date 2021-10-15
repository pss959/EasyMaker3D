//-----------------------------------------------------------------------------
// Product:     OpenCTM tools
// File:        off.h
// Description: Interface for the OFF file format importer/exporter.
//-----------------------------------------------------------------------------
// Copyright (c) 2009-2010 Marcus Geelnard
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//     1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//
//     2. Altered source versions must be plainly marked as such, and must not
//     be misrepresented as being the original software.
//
//     3. This notice may not be removed or altered from any source
//     distribution.
//-----------------------------------------------------------------------------

#ifndef __OFF_H_
#define __OFF_H_

#include <iostream>

#include "mesh.h"
#include "convoptions.h"

/// Import a mesh from an OFF stream.
void Import_OFF(std::istream &s, Mesh * aMesh);

/// Export a mesh to an OFF stream.
void Export_OFF(std::ostream &s, Mesh * aMesh, Options &aOptions);

/// Import a mesh from an OFF file.
void Import_OFF(const char * aFileName, Mesh * aMesh);

/// Export a mesh to an OFF file.
void Export_OFF(const char * aFileName, Mesh * aMesh, Options &aOptions);

#endif // __OFF_H_
