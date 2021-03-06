///////////////////////////////////////////////////////////
//  Merger.cpp
//  Implementation of the Class Merger
//  Created on:      19-???-2009 12:27:54
///////////////////////////////////////////////////////////

#include "Merger.h"
#include "Parser.h"
#include "OverlayDocumentParser.h"
#include "Exception.h"

#include <map>
#include <iostream>

using namespace merge_lib;

Parser Merger::_parser;

Merger::Merger():_baseDocuments(),_overlayDocument(0)
{

}



Merger::~Merger()
{
   std::map<std::string, Document *>::iterator docIterator = _baseDocuments.begin();
   for(; docIterator != _baseDocuments.end(); ++docIterator)
   {
      delete (*docIterator).second;
   }
   if( _overlayDocument )
   {
      delete _overlayDocument;
      _overlayDocument = 0;
   }
   _baseDocuments.clear();
}

void Merger::addBaseDocument(const char * docName)
{
   //if docName has been already opened then do nothing
   if(_baseDocuments.count(docName))
      return;
   Document * newBaseDoc = _parser.parseDocument(docName);
   _baseDocuments.insert(std::pair<std::string, Document *>(docName, newBaseDoc));
}

void Merger::addOverlayDocument(const char * docName)
{
   if( _overlayDocument )
   {
      delete _overlayDocument;
      _overlayDocument = 0;
   }
   if( !_overlayDocument )
   {
      OverlayDocumentParser overlayDocParser;
      _overlayDocument = overlayDocParser.parseDocument(docName);
      if( !_overlayDocument )
      {
         throw Exception("Error loading overlay document!");
      }
   }
}

// The main method which performs the merge
void Merger::merge(const char * overlayDocName, const MergeDescription & pagesToMerge)
{
   if( !_overlayDocument)
   {
      addOverlayDocument(overlayDocName);
      if( !_overlayDocument )
      {
         throw Exception("Error loading overlay document!");
      }
   }
   MergeDescription::const_iterator pageIterator = pagesToMerge.begin();
   for(; pageIterator != pagesToMerge.end(); ++pageIterator )
   {            
      Page * destinationPage = _overlayDocument->getPage( (*pageIterator).overlayPageNumber);
      if( destinationPage == 0 )
      {
         std::stringstream error;
         error << "There is no page with " << (*pageIterator).overlayPageNumber << 
               " number in " << overlayDocName;
         throw Exception(error);
      }
      Document * sourceDocument = _baseDocuments[(*pageIterator).baseDocumentName];
      Page * sourcePage = (sourceDocument == 0)? 0 : sourceDocument->getPage((*pageIterator).basePageNumber);
      bool isPageDuplicated = false;
      if( sourcePage )
      {
         unsigned int howManyTimesPageFound(0);
         for(size_t i = 0; i < pagesToMerge.size(); ++i)
         {
            if(pagesToMerge[i].basePageNumber == (*pageIterator).basePageNumber)
               ++howManyTimesPageFound;
            if(howManyTimesPageFound == 2)
               break;
         }
         isPageDuplicated = (2 == howManyTimesPageFound) ? true : false;
      }

      destinationPage->merge(sourcePage, _overlayDocument, const_cast<MergePageDescription&>((*pageIterator)), isPageDuplicated);
   }

}
// Method performs saving of merged documents into selected file
void Merger::saveMergedDocumentsAs(const char * outDocumentName)
{
   _overlayDocument->saveAs(outDocumentName);
}

