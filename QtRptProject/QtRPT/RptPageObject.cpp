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

#include "RptPageObject.h"

RptPageObject::RptPageObject() {
    this->orientation=0;
    this->ph=1188;
    this->pw=840;
    this->ml=40;
    this->mr=40;
    this->mt=40;
    this->mb=40;
}

void RptPageObject::setProperty(QtRPT *qtrpt, QDomElement docElem) {
    ph = docElem.attribute("pageHeight").toInt();
    pw = docElem.attribute("pageWidth").toInt();
    ml = docElem.attribute("marginsLeft").toInt();
    mr = docElem.attribute("marginsRight").toInt();
    mt = docElem.attribute("marginsTop").toInt();
    mb = docElem.attribute("marginsBottom").toInt();
    orientation = docElem.attribute("orientation").toInt();
    pageNo = docElem.attribute("pageNo").toInt();
    //---
    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if ((!e.isNull()) && (e.tagName() == "ReportBand")) {
            RptBandObject *bandObject = new RptBandObject();
            bandObject->parentReportPage = this;
            bandObject->setProperty(qtrpt,e);
            bandList.append(bandObject);
        }
        n = n.nextSibling();
    }
}

void RptPageObject::addBand(RptBandObject *band) {
    band->parentReportPage = this;
    bandList.append(band);
}

RptBandObject *RptPageObject::getBand(BandType type) {
    RptBandObject *result = 0;
    for (int i=0; i<bandList.size(); i++)
        if (bandList.at(i)->type == type)
            result = bandList.at(i);
    return result;
}

RptFieldObject *RptPageObject::findFieldObjectByName(QString name) {
    for (int i=0; i<bandList.size(); i++)
        for (int j=0; j<bandList.at(i)->fieldList.size(); j++)
            if (bandList.at(i)->fieldList.at(j)->name == name)
                return bandList.at(i)->fieldList.at(j);
    return 0;
}

RptPageObject::~RptPageObject() {
    for (int i=0; i<bandList.size(); i++)
        delete bandList.at(i);
    bandList.clear();
}

QDebug operator<<(QDebug dbg, const RptPageObject &obj) {
    dbg << "Report #" << obj.pageNo  << obj.bandList;
    return dbg;
}
