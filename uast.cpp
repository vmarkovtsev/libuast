#include "uast.h"
#include "roles.h"

#define GET(__NODE__, __PROP__, ...) (impl.__PROP__(__NODE__, ##__VA_ARGS__))

#define check_ptr(__V__)                                     \
  if (__V__ == NULL) {                                       \
    throw std::runtime_error("missing node implementation"); \
  }

NodeAPI::NodeAPI(NodeImpl impl) : impl(impl) {
  checkImpl();
  xmlInitParser();
}

NodeAPI::~NodeAPI() { xmlCleanupParser(); }

void NodeAPI::checkImpl() {
  check_ptr(impl.internal_type);
  check_ptr(impl.token);
  check_ptr(impl.children_size);
  check_ptr(impl.children);
  check_ptr(impl.roles_size);
  check_ptr(impl.roles);
  check_ptr(impl.properties_size);
  check_ptr(impl.properties);
}

std::vector<void *> NodeAPI::find(void *node, std::string query) {
  auto doc = create_document(node);
  auto ctx = create_xpath_context(doc);
  auto xpathObj = evaluate_xpath(ctx, query);

  xmlNodeSetPtr result = xpathObj->nodesetval;
  int size = (result) ? result->nodeNr : 0;
  auto nodes = result->nodeTab;

  std::vector<void *> uast;
  uast.reserve(size);
  for (int i = 0; i < size; i++) {
    uast.push_back(nodes[i]->_private);
  }

  xmlFreeDoc(doc);
  xmlXPathFreeContext(ctx);
  xmlXPathFreeObject(xpathObj);

  return move(uast);
}

xmlDocPtr NodeAPI::create_document(void *node) {
  auto doc = xmlNewDoc(BAD_CAST("1.0"));
  auto xmlNode = create_xml_node(node, NULL);
  xmlDocSetRootElement(doc, xmlNode);
  return doc;
}

xmlXPathContextPtr NodeAPI::create_xpath_context(xmlDocPtr doc) {
  xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
  if (ctx == NULL) {
    throw std::runtime_error("unable to create new XPath context");
  }
  return ctx;
}

xmlXPathObjectPtr NodeAPI::evaluate_xpath(xmlXPathContextPtr ctx,
                                          std::string query) {
  xmlXPathObjectPtr obj = xmlXPathEvalExpression(BAD_CAST(query.data()), ctx);
  if (obj == NULL) {
    throw std::runtime_error("unable to evaluate xpath expression");
  }
  return obj;
}

xmlNodePtr NodeAPI::create_xml_node(void *node, xmlNodePtr parent) {
  auto internal_type = GET(node, internal_type);
  xmlNodePtr xmlNode = xmlNewNode(NULL, BAD_CAST(internal_type));
  xmlNode->_private = node;
  if (parent) {
    xmlAddChild(parent, xmlNode);
  }

  // Token
  auto token = GET(node, token);
  if (!token) {
    xmlNewProp(xmlNode, BAD_CAST("token"), BAD_CAST(token));
  }

  // Token
  auto roles_size = GET(node, roles_size);
  for (int i = 0; i < roles_size; i++) {
    auto role = GET(node, roles, i);
    xmlNewProp(xmlNode, BAD_CAST(role_name_for_id(role)), NULL);
  }

  // Recursivelly visit all children
  auto children_size = GET(node, children_size);
  for (int i = 0; i < children_size; i++) {
    auto child = GET(node, children, i);
    create_xml_node(child, xmlNode);
  }

  return xmlNode;
}
