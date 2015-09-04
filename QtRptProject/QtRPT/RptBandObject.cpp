/*
Name: QtRpt
Version: 1.5.2
Web-site: http://www.qtrpt.tk
Programmer: Aleksey Osipov
E-mail: aliks-os@ukr.net
Web-site: http://www.aliks-os.tk

Copyright 2012-2015 Aleksey Osipov

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "RptBandObject.h"

void RptBandObject::setProperty(QtRPT *qtrpt, QDomElement docElem) {
  name = docElem.attribute("name");
  realHeight = docElem.attribute("height").toInt();
  height = docElem.attribute("height").toInt();
  width = docElem.attribute("width").toInt();
  left = docElem.attribute("left").toInt();
  right = docElem.attribute("right").toInt();
  groupingField = docElem.attribute("groupingField");
  showInGroup = docElem.attribute("showInGroup", "0").toInt();
  startNewPage = docElem.attribute("startNewPage", "0").toInt();

  if (docElem.attribute("type") == "ReportTitle") type = ReportTitle;
  if (docElem.attribute("type") == "PageHeader") type = PageHeader;
  if (docElem.attribute("type") == "MasterData") type = MasterData;
  if (docElem.attribute("type") == "PageFooter") type = PageFooter;
  if (docElem.attribute("type") == "ReportSummary") type = ReportSummary;
  if (docElem.attribute("type") == "MasterFooter") type = MasterFooter;
  if (docElem.attribute("type") == "MasterHeader") type = MasterHeader;
  if (docElem.attribute("type") == "DataGroupHeader") type = DataGroupHeader;
  if (docElem.attribute("type") == "DataGroupFooter") type = DataGroupFooter;
  //---
  QDomNode n = docElem.firstChild();
  while (not n.isNull()) {
    QDomElement e = n.toElement();
    RptFieldObject *fieldObject = new RptFieldObject();
    fieldObject->parentBand = this;
    fieldObject->setProperty(qtrpt, e);
    fieldList.append(fieldObject);

    n = n.nextSibling();
  }
}

void RptBandObject::addField(RptFieldObject *field) {
  field->parentBand = this;
  fieldList.append(field);
}

RptBandObject::~RptBandObject() {
  for (int i = 0; i < fieldList.size(); i++)
    delete fieldList.at(i);
  fieldList.clear();
}

QDebug operator<<(QDebug dbg, const RptBandObject &obj) {
  dbg << obj.name << obj.fieldList;
  return dbg;
}
