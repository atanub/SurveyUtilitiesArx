//The following example creates a new dictionary (ASDK_DICT) and adds it to the named object dictionary. Then it creates two new objects of the custom class AsdkMyClass (derived from AcDbObject) and adds them to the dictionary using setAt(). 

 

//Note You need to close the objects after adding them with setAt().

// Creates two objects of class "AsdkMyClass" fills them in
// with the integers 1 & 2 and then adds them to the
// dictionary associated with the key "ASDK_DICT".  If this
// dictionary doesn't exist, then it's created and added
// to the named objects dictionary.
//
void
createDictionary()
{
    AcDbDictionary *pNamedobj;
    acdbCurDwg()->getNamedObjectsDictionary(pNamedobj,
        AcDb::kForWrite);

    // Check to see if the dictionary we want to create is

    // already present if it's not, then create it and add
    // it to the named object dictionary
    //
    AcDbDictionary *pDict;
    if (pNamedobj->getAt("ASDK_DICT", (AcDbObject*&) pDict,
        AcDb::kForWrite) == Acad::eKeyNotFound)
    {
        pDict = new AcDbDictionary;
        AcDbObjectId DictId;
        pNamedobj->setAt("ASDK_DICT", pDict, DictId);
    }
    pNamedobj->close();

    if (pDict) {
        // Create new objects to add to the new dictionary,

        // add them, then close them.
        //
        AsdkMyClass *pObj1 = new AsdkMyClass(1);
        AsdkMyClass *pObj2 = new AsdkMyClass(2);

        AcDbObjectId rId1, rId2;
        pDict->setAt("OBJ1", pObj1, rId1);
        pDict->setAt("OBJ2", pObj2, rId2);

        pObj1->close();
        pObj2->close();
        pDict->close();
    }

}
//The iterator class for dictionaries is AcDbDictionaryIterator. The following code excerpt obtains a dictionary (ASDK_DICT) from the named object dictionary. It then uses a dictionary iterator to step through the dictionary entries and print the value of the stored integer. Finally, it deletes the iterator and closes the dictionary.

 void
iterateDictionary()
{
    AcDbDictionary *pNamedobj;
    acdbCurDwg()->getNamedObjectsDictionary(pNamedobj,
        AcDb::kForRead);

    // Get a pointer to the ASDK_DICT dictionary
    //
    AcDbDictionary *pDict;
    pNamedobj->getAt("ASDK_DICT", (AcDbObject*&)pDict,
        AcDb::kForRead);

    pNamedobj->close();

    // Get an iterator for the ASDK_DICT dictionary
    //
    AcDbDictionaryIterator* pDictIter= pDict->newIterator();

    AsdkMyClass *pMyCl;
    Adesk::Int16 val;
    for (; !pDictIter->done(); pDictIter->next()) {

        // Get the current record, open it for read and
        // print it's data.
        //
        pDictIter->getObject((AcDbObject*&)pMyCl,
            AcDb::kForRead);
        pMyCl->getData(val);
        pMyCl->close();
        ads_printf("\nintval is:  %d", val);
    }
    delete pDictIter;
    pDict->close();

}