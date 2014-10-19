/*
 * xml-base.h
 */

#ifndef __XML_BASE_H
#define __XML_BASE_H

#include <tinyxml.h>

#include <string>


class cXmlBase {
private:
        std::string path, root_element;

        TiXmlDocument *document;
        TiXmlElement *root;
protected:
        virtual ~cXmlBase();

        virtual void copy_to_objects(void) {}
public:
     cXmlBase(const char *_root_element);

        bool load(const std::string &_path);
        void clear(void);

        void add_subelement(TiXmlElement &main_element,
                const char *name, const std::string &text);

        TiXmlDocument *get_document(void) { return document; }
        TiXmlElement *get_root(void) { return root; }
        void set_root(void);
};

#endif /* __XML_BASE_H */
