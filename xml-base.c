/*
 * xml-base.c
 */

#include "xml-base.h"

#include <vdr/tools.h>

#include <unistd.h>


using namespace std;
using namespace a_land;

/* --- cXmlBase ------------------------------------------------------------- */

cXmlBase::cXmlBase(const char *_root_element)
{
	root_element = _root_element;
        document = NULL;
        root = NULL;
}


cXmlBase::~cXmlBase()
{
        if (document) {
                document->SaveFile();
        }

        delete document;
}


bool cXmlBase::load(const string &_path)
{
        if (document)
                return true;

	path = _path;

        dsyslog("[audiorecorder]: loading xml-file (%s) (%s ,%s())",
                path.c_str(), __FILE__, __func__);

        document = new TiXmlDocument(path);

        if (access(path.c_str(), F_OK) == -1) {
                dsyslog("[audiorecorder]: creating empty xml-file (%s) (%s ,"
                        "%s())", path.c_str(), __FILE__, __func__);
                TiXmlElement new_root(root_element);
                document->InsertEndChild(new_root);
                document->SaveFile();
        }

        if (! document->LoadFile()) {
                dsyslog("[audiorecorder]: error while parsing xml-file (%s) "
                        "(%s, %s())", path.c_str(), __FILE__, __func__);
                dsyslog("[audiorecorder]: %s, row: %d, column: %d (%s, %s())",
                        document->ErrorDesc(), document->ErrorRow(),
                        document->ErrorCol(), __FILE__, __func__);
                return false;
        }

        set_root();

        if (! root || root->ValueStr() != root_element)
                return false;

        copy_to_objects();

        return true;
}


void cXmlBase::clear(void)
{
        remove(path.c_str());

        delete document;
        document = new TiXmlDocument(path);

        root = new TiXmlElement(root_element);
}


void cXmlBase::add_subelement(TiXmlElement &main_element, const char *name,
        const string &text)
{
        if (! name || text.empty())
                return;

        TiXmlElement element(name);
        TiXmlText txt(text);
        element.InsertEndChild(txt);

        main_element.InsertEndChild(element);
}


void cXmlBase::set_root(void)
{
        if (document)
                root = document->RootElement();
}
