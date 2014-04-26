/*
 * xml-base.h
 */

#ifndef __XML_BASE_H
#define __XML_BASE_H

#include "tinyxml/tinyxml.h"

#include <string>


class cXmlBase {
private:
        std::string path, root_element;

        a_land::TiXmlDocument *document;
        a_land::TiXmlElement *root;
protected:
	virtual ~cXmlBase();

        virtual void copy_to_objects(void) {}
public:
	cXmlBase(const char *_root_element);

        bool load(const std::string &_path);
        void clear(void);

        void add_subelement(a_land::TiXmlElement &main_element,
                const char *name, const std::string &text);

        a_land::TiXmlDocument *get_document(void) { return document; }
        a_land::TiXmlElement *get_root(void) { return root; }
        void set_root(void);
};

#endif /* __XML_BASE_H */
