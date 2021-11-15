//-----------------------------------------------------------------------------
// Product:     OpenCTM tools
// File:        dae.cpp
// Description: Implementation of the DAE (Collada) file format
//              importer/exporter.
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

#include <stdexcept>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <clocale>
#include <ctime>
#include <tinyxml2.h>
#include "dae.h"

#if !defined(WIN32) && defined(_WIN32)
#define WIN32
#endif
#ifdef WIN32
#include <windows.h>
#endif


using namespace std;
using tinyxml2::XMLDocument;
using tinyxml2::XMLElement;
using tinyxml2::XMLHandle;
using tinyxml2::XMLPrinter;

enum Axis
{
  X,Y,Z,S,T
};

class Source
{
public:
  Source() : stride(0), count(0), offset(0)
  {
  }
  
  Source(const Source& copy) : array(copy.array), stride(copy.stride), count(copy.count), offset(copy.offset), params(copy.params)
  {
  }
  
  vector<float> array;
  size_t stride, count, offset;
  vector<string> params;

};

class Indexes {
public:
	Indexes(size_t _vertIndex = 0, size_t _normalIndex = 0, size_t _texcoordIndex = 0) : vertIndex(_vertIndex), normalIndex(_normalIndex), texcoordIndex(_texcoordIndex) {
		
	}
	
	Indexes(const Indexes& copy) : vertIndex(copy.vertIndex), normalIndex(copy.normalIndex), texcoordIndex(copy.texcoordIndex) {
		
	}
	
	size_t vertIndex, normalIndex, texcoordIndex;
};

enum Semantic
{
  VERTEX,
  NORMAL,
  TEXCOORD,
  POSITIONS,
  UNKNOWN
};

struct Input
{
  string source;
  Semantic semantic;
  size_t offset;
};

Semantic ToSemantic(const string& semantic)
{
  if (semantic == "VERTEX")
    return VERTEX;
  else if (semantic == "NORMAL")
    return NORMAL;
  else if (semantic == "TEXCOORD")
    return TEXCOORD;
  else if (semantic == "POSITIONS")
    return POSITIONS;
  else
    return UNKNOWN;
}

void ReadIndexArray(XMLElement* p , vector<size_t>& array)
{
  istringstream strStream (p->GetText());
  char val[100];
  size_t value = 0;
  while (!strStream.eof()) {
    strStream >> val;
    value = atoi(val);
    array.push_back(value);
  }
}

void ReadInputs(XMLElement* rootElem,bool& hasVerts,bool& hasNormals,bool& hasTexcoords, string& vertSource,string& normalSource,string& texcoordSource, vector<Input>& inputs) {
	XMLHandle root(rootElem);
	for(XMLElement* inputElem = root.FirstChildElement( "input" ).ToElement();inputElem; inputElem = inputElem->NextSiblingElement())
	{
		if(string(inputElem->Value()) != "input")
			continue;
		//XMLHandle input(inputElem);
		inputs.push_back(Input());
		inputs.back().source = string(inputElem->Attribute("source")).substr(1);
		inputs.back().offset = atoi(inputElem->Attribute("offset"));
		inputs.back().semantic = ToSemantic(inputElem->Attribute("semantic"));
		switch(inputs.back().semantic)
		{
			case VERTEX:
                hasVerts = true;
                vertSource = inputs.back().source;
                break;
			case NORMAL:
                hasNormals = true;
                normalSource = inputs.back().source;
                break;
			case TEXCOORD:
                hasTexcoords = true;
                texcoordSource = inputs.back().source;
                break;
			default:
                break;
		}
	}
}

Source& GetSource(map<string, Source >& sources, map<string, vector<Input> >& vertices,const string& source)
{
  map<string, Source >::iterator srcIterator = sources.find(source);
  if (srcIterator != sources.end())
    return srcIterator->second;
  map<string, vector<Input> >::iterator vertIterator = vertices.find(source);
  if (vertIterator != vertices.end() ) {
    for (vector<Input>::iterator i = vertIterator->second.begin(); i != vertIterator->second.end() ; ++i) {
      srcIterator = sources.find(i->source);
      if (srcIterator != sources.end())
        return srcIterator->second;
      
    }
  } else {
    throw string("Error");
  }
  
  return srcIterator->second;
}

void InsertVertNormalTexcoord(vector<Vector3>& vertVector,vector<Vector3>& normalVector,vector<Vector2>& texcoordVector, bool hasVerts, bool hasNormals, bool hasTexcoords,const string& vertSource ,const string& normalSource ,const string& texcoordSource ,size_t vertIndex , size_t normalIndex , size_t texcoordIndex, map<string, Source >& sources,map<string, vector<Input> >& vertices)
{
  if (hasVerts) {
    Source& src = GetSource(sources, vertices , vertSource);
    float x = 0, y = 0, z = 0;
    if (src.stride >= 1)
      x = src.array[src.offset + vertIndex*src.stride];
    if (src.stride >= 2)
      y = src.array[src.offset + vertIndex*src.stride + 1];
    if (src.stride >= 3)
      z = src.array[src.offset + vertIndex*src.stride + 2];
    vertVector.push_back(Vector3(x,y,z));
  }
  
  if (hasNormals) {
    Source& src = GetSource(sources, vertices , normalSource);
    float x = 0, y = 0, z = 0;
    if (src.stride >= 1)
      x = src.array[src.offset + normalIndex*src.stride];
    if (src.stride >= 2)
      y = src.array[src.offset + normalIndex*src.stride + 1];
    if (src.stride >= 3)
      z = src.array[src.offset + normalIndex*src.stride + 2];
    normalVector.push_back(Vector3(x,y,z) );
  }
  
  if (hasTexcoords) {
    Source& src = GetSource(sources, vertices , texcoordSource);
    float s = 0, t = 0;
    if (src.stride >= 1)
      s = src.array[src.offset + texcoordIndex*src.stride];
    if (src.stride >= 2)
      t = src.array[src.offset + texcoordIndex*src.stride + 1];

    texcoordVector.push_back(Vector2(s,t));
  }
}

/// Forward reference so that the file and stream loaders can use this function.
void Import_DAE(XMLDocument &doc, Mesh * aMesh);

/// Import a DAE file from a stream.
void Import_DAE(std::istream &f, Mesh * aMesh)
{
  // Start by ensuring that we use proper locale settings for the file format
  setlocale(LC_NUMERIC, "C");

  const std::string data((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());

  // Load the XML document
  XMLDocument doc;
  if (!doc.Parse(data.data()))
    Import_DAE(doc, aMesh);
}

/// Import a DAE file from a file.
void Import_DAE(const char * aFileName, Mesh * aMesh)
{
  // Start by ensuring that we use proper locale settings for the file format
  setlocale(LC_NUMERIC, "C");

  // Load the XML document
  XMLDocument doc;
  if (!doc.LoadFile(aFileName))
    Import_DAE(doc, aMesh);
  else
    throw runtime_error("Could not open input file.");
}

/// Import a DAE file from a file.
void Import_DAE(XMLDocument &doc, Mesh * aMesh)
{
  // Start by ensuring that we use proper locale settings for the file format
  setlocale(LC_NUMERIC, "C");

  // Clear the mesh
  aMesh->Clear();

  XMLHandle hDoc(&doc);
  XMLElement* elem = hDoc.FirstChildElement().ToElement();
  XMLHandle hRoot(elem);

  map<string, Source > sources;
  size_t indicesOffset = 0, vertexOffset = 0, texcoordOffset = 0, normalOffset = 0;

  XMLHandle geometry = hRoot.FirstChildElement( "library_geometries" ).FirstChildElement("geometry");
  for(elem = geometry.ToElement(); elem; elem=elem->NextSiblingElement())
  {
    XMLHandle geometry(elem);

    XMLElement* meshElem =  geometry.FirstChildElement("mesh").ToElement();

    if(meshElem)
    {
      XMLHandle mesh(meshElem);

      XMLElement* sourceElem;
      for(sourceElem = mesh.FirstChildElement("source").ToElement(); sourceElem;
          sourceElem = sourceElem->NextSiblingElement())
      {
        if(string(sourceElem->Value()) != "source")
          continue;
        XMLHandle source(sourceElem);
        string id = source.ToElement()->Attribute("id");
        XMLElement* arr = sourceElem->FirstChildElement("float_array")->ToElement();
        string str = arr->GetText();
        istringstream strStream (str);
        sources.insert(make_pair(id, Source()));

        XMLElement* techniqueElem = sourceElem->FirstChildElement("technique_common")->ToElement();
        XMLElement* accessorElem = techniqueElem->FirstChildElement("accessor")->ToElement();

        sources[id].stride = atoi(accessorElem->Attribute("stride"));
        sources[id].count = atoi(accessorElem->Attribute("count"));
    if (accessorElem->Attribute("offset"))
      sources[id].offset = atoi(accessorElem->Attribute("offset"));

        string val;
        float value = 0;
        while(!strStream.eof())
        {
          strStream >> val;
          value = std::stof(val);
          sources[id].array.push_back(value);
        }
      }

      XMLElement* verticesElem = mesh.FirstChildElement("vertices").ToElement();
      map<string, vector<Input> > vertices;
      if (verticesElem) {
        string id = verticesElem->Attribute("id");
        vertices.insert(make_pair(id, vector<Input>()));
        XMLElement* inputElem;
        for(inputElem = verticesElem->FirstChildElement("input")->ToElement();
            inputElem; inputElem = inputElem->NextSiblingElement())
        {
          if(string(inputElem->Value()) != "input")
            continue;

          vertices[id].push_back(Input());
          vertices[id].back().source = string(inputElem->Attribute("source")).substr(1);
          vertices[id].back().semantic = ToSemantic(inputElem->Attribute("semantic"));
        }
      }

      XMLElement* trianglesElem = mesh.FirstChildElement("triangles").ToElement();
      if(trianglesElem)
      {
        XMLHandle triangles(trianglesElem);
        vector<Input> inputs;
        bool hasVerts = false, hasNormals = false, hasTexcoords = false;
        string vertSource = "", normalSource = "", texcoordSource = "";
    /*
        XMLElement* inputElem;
        for(inputElem = triangles.FirstChild( "input" ).ToElement();
            inputElem; inputElem = inputElem->NextSiblingElement())
        {
          if(string(inputElem->Value()) != "input")
            continue;
          //XMLHandle input(inputElem);
          inputs.push_back(Input());
          inputs.back().source = string(inputElem->Attribute("source")).substr(1);
          inputs.back().offset = atoi(inputElem->Attribute("offset"));
          inputs.back().semantic = ToSemantic(inputElem->Attribute("semantic"));
          switch(inputs.back().semantic)
          {
            case VERTEX:
              hasVerts = true;
              vertSource = inputs.back().source;
              break;
            case NORMAL:
              hasNormals = true;
              normalSource = inputs.back().source;
              break;
            case TEXCOORD:
              hasTexcoords = true;
              texcoordSource = inputs.back().source;
              break;
            default:
              break;
          }
        }
    */
    ReadInputs(trianglesElem, hasVerts, hasNormals, hasTexcoords, vertSource, normalSource, texcoordSource, inputs);

        vector<size_t> pArray;
        XMLElement* p = triangles.FirstChildElement( "p" ).ToElement();

        ReadIndexArray(p,pArray);

        vector<size_t> indexVector;
        vector<Vector3> vertVector, normalVector;
        vector<Vector2> texcoordVector;
        map<size_t, map<size_t, map< size_t, size_t > > > prevIndices;
        size_t index = 0;
        for (size_t i = 0; i < pArray.size() ; i += inputs.size()) {
          size_t vertIndex = 0, normalIndex = 0, texcoordIndex = 0;
          for (vector<Input>::const_iterator j = inputs.begin(); j != inputs.end(); ++j) {
            switch (j->semantic) {
              case VERTEX:
                vertIndex = pArray[i + j->offset];
                break;
              case NORMAL:
                normalIndex = pArray[i + j->offset];
                break;
              case TEXCOORD:
                texcoordIndex = pArray[i + j->offset];
                break;
              default:
                break;
            }
          }
          map<size_t, map<size_t, map< size_t, size_t > > >::iterator prevIt1 = prevIndices.find(vertIndex);

          if(prevIt1 != prevIndices.end())
          {
            map<size_t, map< size_t, size_t > >::iterator prevIt2 = prevIt1->second.find(normalIndex);
            if(prevIt2 != prevIt1->second.end())
            {
              map< size_t, size_t >::iterator prevIt3 = prevIt2->second.find(texcoordIndex);
              if(prevIt3 != prevIt2->second.end())
              {
                indexVector.push_back(prevIt3->second);
              }
              else
              {
                indexVector.push_back(index);
                prevIt2->second.insert(make_pair(texcoordIndex, index));
                InsertVertNormalTexcoord(vertVector, normalVector, texcoordVector, hasVerts, hasNormals, hasTexcoords, vertSource, normalSource, texcoordSource, vertIndex, normalIndex, texcoordIndex, sources, vertices);
                ++index;
              }
            }
            else
            {
              indexVector.push_back(index);
              prevIt1->second.insert(make_pair(normalIndex, map< size_t, size_t >()));
              prevIt1->second[normalIndex].insert(make_pair(texcoordIndex, index));
              InsertVertNormalTexcoord(vertVector, normalVector, texcoordVector, hasVerts, hasNormals, hasTexcoords, vertSource, normalSource, texcoordSource, vertIndex, normalIndex, texcoordIndex, sources, vertices);
              ++index;
            }
          }
          else
          {
            indexVector.push_back(index);
            prevIndices.insert(make_pair(vertIndex,map<size_t, map< size_t, size_t > >()));
            prevIndices[vertIndex].insert(make_pair(normalIndex, map< size_t, size_t >()));
            prevIndices[vertIndex][normalIndex].insert(make_pair(texcoordIndex, index));
            InsertVertNormalTexcoord(vertVector, normalVector, texcoordVector, hasVerts, hasNormals, hasTexcoords, vertSource, normalSource, texcoordSource, vertIndex, normalIndex, texcoordIndex, sources, vertices);
            ++index;
          }

        }

    XMLElement* polylistElem = mesh.FirstChildElement("polylist").ToElement();

    if (polylistElem) {
      XMLHandle polylist(polylistElem);
      vector<size_t> vcountArray, pArray;
      XMLElement* vcountElem = polylist.FirstChildElement("vcount").ToElement();
      ReadIndexArray(vcountElem, vcountArray);
      XMLElement* pElem = polylist.FirstChildElement("p").ToElement();
      ReadIndexArray(pElem, pArray);
      vector<Input> inputs;
      bool hasVerts = false, hasNormals = false, hasTexcoords = false;
      string vertSource = "", normalSource = "", texcoordSource = "";

      ReadInputs(polylistElem, hasVerts, hasNormals, hasTexcoords, vertSource, normalSource, texcoordSource, inputs);
      size_t offset = 0;
      for (size_t i = 0; i < vcountArray.size(); ++i) {
        vector<Indexes> convexPolygon;
        for (size_t j = 0; j < vcountArray[i]; ++j) {
          convexPolygon.push_back(Indexes());
          for (vector<Input>::const_iterator j = inputs.begin(); j != inputs.end(); ++j) {
            switch (j->semantic) {
              case VERTEX:
                convexPolygon.back().vertIndex = pArray[offset + j->offset];
                break;
              case NORMAL:
                convexPolygon.back().normalIndex = pArray[offset + j->offset];
                break;
              case TEXCOORD:
                convexPolygon.back().texcoordIndex = pArray[offset + j->offset];
                break;
              default:
                break;
            }
          }
        }
        offset += vcountArray[i];
      }



    }

        size_t indicesOff = indicesOffset, vertexOff = vertexOffset, normalOff = normalOffset, texcoordOff = texcoordOffset;
        indicesOffset += indexVector.size();
        vertexOffset += vertVector.size();
        normalOffset += normalVector.size();
        texcoordOffset += texcoordVector.size();
        aMesh->mIndices.resize(indicesOffset );
        aMesh->mVertices.resize(vertexOffset );
        aMesh->mNormals.resize(normalOffset );
        aMesh->mTexCoords.resize(texcoordOffset );

        for(size_t i = 0; i < indexVector.size(); ++i)
          aMesh->mIndices[indicesOff + i] = indexVector[i];

        for(size_t i = 0; i < vertVector.size(); ++i)
          aMesh->mVertices[vertexOff + i] = vertVector[i];

        for(size_t i = 0; i < normalVector.size(); ++i)
          aMesh->mNormals[normalOff + i] = normalVector[i];

        for(size_t i = 0; i < texcoordVector.size(); ++i)
          aMesh->mTexCoords[texcoordOff + i] = texcoordVector[i];
      }
    }
  }
}

/// Dump a float array to an XML text node.
static void FloatArrayToXML(XMLElement * aNode, float * aArray,
  unsigned int aCount)
{
  stringstream ss;
  for(unsigned int i = 0; i < aCount; ++ i)
    ss << aArray[i] << " ";
  XMLDocument doc;
  aNode->LinkEndChild(doc.NewText(ss.str().c_str()));
}

/// Generate an ISO 8601 format date string.
static string MakeISO8601DateTime(void)
{
  char buf[500];
#ifdef WIN32
  SYSTEMTIME tm;
  GetSystemTime(&tm);
  sprintf(buf, "%i-%02i-%02iT%02i:%02i:%02i.%03iZ", tm.wYear,
          tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond,
          tm.wMilliseconds);
#else
  time_t t;
  time(&t);
  struct tm tm;
  localtime_r(&t, &tm);
  sprintf(buf, "%i-%02i-%02iT%02i:%02i:%02i", tm.tm_year + 1900,
          tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
#endif
  return string(buf);
}

/// Forward reference so that the file and stream loaders can use this function.
void Export_DAE(XMLDocument &xmlDoc, Mesh * aMesh, Options &aOptions);

void Export_DAE(std::ostream &s, Mesh * aMesh, Options &aOptions) {
  // Create the XML document
  XMLDocument doc;
  Export_DAE(doc, aMesh, aOptions);

  // Write the document to the stream.
  XMLPrinter printer;
  doc.Accept(&printer);
  s << printer.CStr();
}

/// Export a DAE file to a file.
void Export_DAE(const char * aFileName, Mesh * aMesh, Options &aOptions)
{
  // Start by ensuring that we use proper locale settings for the file format
  setlocale(LC_NUMERIC, "C");

  XMLDocument xmlDoc;
  Export_DAE(xmlDoc, aMesh, aOptions);
  // Save the XML documenxt to a file
  xmlDoc.SaveFile(aFileName);
  if(xmlDoc.Error())
    throw runtime_error(string(xmlDoc.ErrorStr()));
}

void Export_DAE(XMLDocument &xmlDoc, Mesh * aMesh, Options &aOptions) {
  XMLElement * elem;
  string dateTime = MakeISO8601DateTime();

  // What should we export?
  bool exportTexCoords = aMesh->HasTexCoords() && !aOptions.mNoTexCoords;
  bool exportNormals = aMesh->HasNormals() && !aOptions.mNoNormals;

  // Set XML declaration
  //xmlDoc.LinkEndChild(new XMLDeclaration("1.0", "utf-8", ""));

  // Create root node
  XMLElement * root = xmlDoc.NewElement("COLLADA");
  xmlDoc.LinkEndChild(root);
  root->SetAttribute("xmlns", "http://www.collada.org/2005/11/COLLADASchema");
  root->SetAttribute("version", "1.4.1");

  // Create traceability nodes
  XMLElement * asset = xmlDoc.NewElement("asset");
  root->LinkEndChild(asset);
  XMLElement * contributor = xmlDoc.NewElement("contributor");
  asset->LinkEndChild(contributor);
  XMLElement * authoring_tool = xmlDoc.NewElement("authoring_tool");
  contributor->LinkEndChild(authoring_tool);
  authoring_tool->LinkEndChild(xmlDoc.NewText("ctmconv"));
  XMLElement * comments = xmlDoc.NewElement("comments");
  contributor->LinkEndChild(comments);
  comments->LinkEndChild(xmlDoc.NewText(aMesh->mComment.c_str()));
  elem = xmlDoc.NewElement("created");
  asset->LinkEndChild(elem);
  elem->LinkEndChild(xmlDoc.NewText(dateTime.c_str()));
  elem = xmlDoc.NewElement("modified");
  asset->LinkEndChild(elem);
  elem->LinkEndChild(xmlDoc.NewText(dateTime.c_str()));

  // Create the geometry nodes
  XMLElement * library_geometries = xmlDoc.NewElement("library_geometries");
  root->LinkEndChild(library_geometries);
  XMLElement * geometry = xmlDoc.NewElement("geometry");
  library_geometries->LinkEndChild(geometry);
  geometry->SetAttribute("id", "Mesh-1");
  geometry->SetAttribute("name", "Mesh-1");
  XMLElement * mesh = xmlDoc.NewElement("mesh");
  geometry->LinkEndChild(mesh);

  // Vertices (positions)
  XMLElement * source_position = xmlDoc.NewElement("source");
  mesh->LinkEndChild(source_position);
  source_position->SetAttribute("id", "Mesh-1-positions");
  source_position->SetAttribute("name", "position");
  XMLElement * positions_array = xmlDoc.NewElement("float_array");
  source_position->LinkEndChild(positions_array);
  positions_array->SetAttribute("id", "Mesh-1-positions-array");
  positions_array->SetAttribute("count", int(aMesh->mVertices.size() * 3));
  FloatArrayToXML(positions_array, &aMesh->mVertices[0].x, aMesh->mVertices.size() * 3);
  XMLElement * positions_technique = xmlDoc.NewElement("technique_common");
  source_position->LinkEndChild(positions_technique);
  XMLElement * positions_technique_accessor = xmlDoc.NewElement("accessor");
  positions_technique->LinkEndChild(positions_technique_accessor);
  positions_technique_accessor->SetAttribute("count", int(aMesh->mVertices.size()));
  positions_technique_accessor->SetAttribute("offset", 0);
  positions_technique_accessor->SetAttribute("source", "#Mesh-1-positions-array");
  positions_technique_accessor->SetAttribute("stride", 3);
  elem = xmlDoc.NewElement("param");
  positions_technique_accessor->LinkEndChild(elem);
  elem->SetAttribute("name", "X");
  elem->SetAttribute("type", "float");
  elem = xmlDoc.NewElement("param");
  positions_technique_accessor->LinkEndChild(elem);
  elem->SetAttribute("name", "Y");
  elem->SetAttribute("type", "float");
  elem = xmlDoc.NewElement("param");
  positions_technique_accessor->LinkEndChild(elem);
  elem->SetAttribute("name", "Z");
  elem->SetAttribute("type", "float");

  // Normals
  if(exportNormals)
  {
    XMLElement * source_normal = xmlDoc.NewElement("source");
    mesh->LinkEndChild(source_normal);
    source_normal->SetAttribute("id", "Mesh-1-normals");
    source_normal->SetAttribute("name", "normal");
    XMLElement * normals_array = xmlDoc.NewElement("float_array");
    source_normal->LinkEndChild(normals_array);
    normals_array->SetAttribute("id", "Mesh-1-normals-array");
    normals_array->SetAttribute("count", int(aMesh->mVertices.size() * 3));
    FloatArrayToXML(normals_array, &aMesh->mNormals[0].x, aMesh->mNormals.size() * 3);
    XMLElement * normals_technique = xmlDoc.NewElement("technique_common");
    source_normal->LinkEndChild(normals_technique);
    XMLElement * normals_technique_accessor = xmlDoc.NewElement("accessor");
    normals_technique->LinkEndChild(normals_technique_accessor);
    normals_technique_accessor->SetAttribute("count", int(aMesh->mVertices.size()));
    normals_technique_accessor->SetAttribute("offset", 0);
    normals_technique_accessor->SetAttribute("source", "#Mesh-1-normals-array");
    normals_technique_accessor->SetAttribute("stride", 3);
    elem = xmlDoc.NewElement("param");
    normals_technique_accessor->LinkEndChild(elem);
    elem->SetAttribute("name", "X");
    elem->SetAttribute("type", "float");
    elem = xmlDoc.NewElement("param");
    normals_technique_accessor->LinkEndChild(elem);
    elem->SetAttribute("name", "Y");
    elem->SetAttribute("type", "float");
    elem = xmlDoc.NewElement("param");
    normals_technique_accessor->LinkEndChild(elem);
    elem->SetAttribute("name", "Z");
    elem->SetAttribute("type", "float");
  }

  // UV map
  if(exportTexCoords)
  {
    XMLElement * source_map1 = xmlDoc.NewElement("source");
    mesh->LinkEndChild(source_map1);
    source_map1->SetAttribute("id", "Mesh-1-map1");
    source_map1->SetAttribute("name", "map1");
    XMLElement * map1_array = xmlDoc.NewElement("float_array");
    source_map1->LinkEndChild(map1_array);
    map1_array->SetAttribute("id", "Mesh-1-map1-array");
    map1_array->SetAttribute("count", int(aMesh->mVertices.size() * 3));
    FloatArrayToXML(map1_array, &aMesh->mTexCoords[0].u, aMesh->mTexCoords.size() * 2);
    XMLElement * map1_technique = xmlDoc.NewElement("technique_common");
    source_map1->LinkEndChild(map1_technique);
    XMLElement * map1_technique_accessor = xmlDoc.NewElement("accessor");
    map1_technique->LinkEndChild(map1_technique_accessor);
    map1_technique_accessor->SetAttribute("count", int(aMesh->mVertices.size()));
    map1_technique_accessor->SetAttribute("offset", 0);
    map1_technique_accessor->SetAttribute("source", "#Mesh-1-map1-array");
    map1_technique_accessor->SetAttribute("stride", 2);
    elem = xmlDoc.NewElement("param");
    map1_technique_accessor->LinkEndChild(elem);
    elem->SetAttribute("name", "S");
    elem->SetAttribute("type", "float");
    elem = xmlDoc.NewElement("param");
    map1_technique_accessor->LinkEndChild(elem);
    elem->SetAttribute("name", "T");
    elem->SetAttribute("type", "float");
  }

  // Vertices
  XMLElement * vertices = xmlDoc.NewElement("vertices");
  mesh->LinkEndChild(vertices);
  vertices->SetAttribute("id", "Mesh-1-vertices");
  XMLElement * vertices_input = xmlDoc.NewElement("input");
  vertices->LinkEndChild(vertices_input);
  vertices_input->SetAttribute("semantic", "POSITION");
  vertices_input->SetAttribute("source", "#Mesh-1-positions");

  // Triangles
  XMLElement * triangles = xmlDoc.NewElement("triangles");
  mesh->LinkEndChild(triangles);
  triangles->SetAttribute("count", int(aMesh->mIndices.size() / 3));
  int triangleInputCount = 0;
  elem = xmlDoc.NewElement("input");
  triangles->LinkEndChild(elem);
  elem->SetAttribute("offset", triangleInputCount);
  elem->SetAttribute("semantic", "VERTEX");
  elem->SetAttribute("source", "#Mesh-1-vertices");
  ++ triangleInputCount;
  if(exportNormals)
  {
    elem = xmlDoc.NewElement("input");
    triangles->LinkEndChild(elem);
    elem->SetAttribute("offset", triangleInputCount);
    elem->SetAttribute("semantic", "NORMAL");
    elem->SetAttribute("source", "#Mesh-1-normals");
    ++ triangleInputCount;
  }
  if(exportTexCoords)
  {
    elem = xmlDoc.NewElement("input");
    triangles->LinkEndChild(elem);
    elem->SetAttribute("offset", triangleInputCount);
    elem->SetAttribute("semantic", "TEXCOORD");
    elem->SetAttribute("source", "#Mesh-1-map1");
    elem->SetAttribute("set", 0);
    ++ triangleInputCount;
  }
  {
    elem = xmlDoc.NewElement("p");
    triangles->LinkEndChild(elem);
    stringstream ss;
    for(unsigned int i = 0; i < aMesh->mIndices.size(); ++ i)
      for(int j = 0; j < triangleInputCount; ++ j)
        ss << aMesh->mIndices[i] << " ";
    elem->LinkEndChild(xmlDoc.NewText(ss.str().c_str()));
  }

  // Scene
  XMLElement * library_visual_scenes = xmlDoc.NewElement("library_visual_scenes");
  root->LinkEndChild(library_visual_scenes);
  XMLElement * visual_scene = xmlDoc.NewElement("visual_scene");
  library_visual_scenes->LinkEndChild(visual_scene);
  visual_scene->SetAttribute("id", "Scene-1");
  visual_scene->SetAttribute("name", "Scene-1");
  XMLElement * visual_scene_node = xmlDoc.NewElement("node");
  visual_scene->LinkEndChild(visual_scene_node);
  visual_scene_node->SetAttribute("id", "Object-1");
  visual_scene_node->SetAttribute("name", "Object-1");
  XMLElement * instance_geometry = xmlDoc.NewElement("instance_geometry");
  visual_scene_node->LinkEndChild(instance_geometry);
  instance_geometry->SetAttribute("url", "#Mesh-1");
  XMLElement * scene = xmlDoc.NewElement("scene");
  root->LinkEndChild(scene);
  XMLElement * instance_visual_scene = xmlDoc.NewElement("instance_visual_scene");
  scene->LinkEndChild(instance_visual_scene);
  instance_visual_scene->SetAttribute("url", "#Scene-1");
}
