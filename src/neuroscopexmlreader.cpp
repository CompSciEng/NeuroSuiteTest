/***************************************************************************
                          neuroscopexmlreader.cpp  -  description
                             -------------------
    begin                : Tue Mar 2 2004
    copyright            : (C) 2004 by Lynn Hazan
    email                : lynn.hazan.myrealbox.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
//application specific include files.
#include "neuroscopexmlreader.h"
#include "tags.h"

#include <QList>

//include files for QT
#include <QFileInfo> 
#include <QString> 
#include <QDomDocument>
#include <QDebug>

using namespace neuroscope;

NeuroscopeXmlReader::NeuroscopeXmlReader()
{
}

NeuroscopeXmlReader::~NeuroscopeXmlReader(){
}

bool NeuroscopeXmlReader::parseFile(const QString& url,fileType type){
    this->type = type;
    qDebug()<<" URL:"<<url;
    QFile input(url);

    QDomDocument docElement;
    QString errorMsg;
    int errorRow;
    int errorCol;
    if ( !docElement.setContent( &input, &errorMsg, &errorRow, &errorCol ) ) {
        qWarning() << "Unable to load document.Parse error in " << url << ", line " << errorRow << ", col " << errorCol << ": " << errorMsg << endl;
        return false;
    }

    QDomElement element = docElement.documentElement();

    if (element.tagName() == QLatin1String("parameters")) {
        if( element.hasAttribute(VERSION)) {
            readVersion = element.attribute(VERSION);
            qDebug()<<" readVersion "<<readVersion;
        }
    }
    documentNode = element;




    // Init libxml
    xmlInitParser();

    // Load XML document
    doc = xmlParseFile(url.toLatin1());
    if(doc == NULL)
        return false;

    // Create xpath evaluation context
    xpathContex = xmlXPathNewContext(doc);
    if(xpathContex == NULL){
        xmlFreeDoc(doc);
        return false;
    }

    //Read the document version
    xmlNodePtr rootElement = xmlDocGetRootElement(doc);
    xmlChar* versionTag = xmlCharStrdup(VERSION.toLatin1());
    if(rootElement != NULL){
        xmlChar* sVersion = xmlGetProp(rootElement,versionTag);//get the attribute with the name versionTag
        if(sVersion != NULL)
            readVersion = QString((char*)sVersion);
        xmlFree(sVersion);
    }
    qDebug()<<" readVersion"<<readVersion;
    xmlFree(versionTag);
    
    return true;
}


void NeuroscopeXmlReader::closeFile(){
    //Cleanup
    xmlXPathFreeContext(xpathContex);
    xmlFreeDoc(doc);
    readVersion.clear();

    //Shutdown libxml
    xmlCleanupParser();
}


int NeuroscopeXmlReader::getResolution()const{
    int resolution = 0;
    QDomNode n = documentNode.firstChild();
    if (!n.isNull()) {
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull()) {
                QString tag = e.tagName();
                if (tag == ACQUISITION) {
                    QDomNode acquisition = e.firstChild(); // try to convert the node to an element.
                    while(!acquisition.isNull()) {
                        QDomElement u = acquisition.toElement();
                        if (!u.isNull()) {
                            tag = u.tagName();
                            if (tag == BITS) {
                                resolution = u.text().toInt();
                                return resolution;
                                break;
                            }
                        }
                        acquisition = acquisition.nextSibling();
                    }
                    break;
                }
            }
            n = n.nextSibling();
        }
    }

    return resolution;
}

int NeuroscopeXmlReader::getNbChannels()const{
    int nbChannels = 0;
    QDomNode n = documentNode.firstChild();
    if (!n.isNull()) {
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull()) {
                QString tag = e.tagName();
                if (tag == ACQUISITION) {
                    QDomNode acquisition = e.firstChild(); // try to convert the node to an element.
                    while(!acquisition.isNull()) {
                        QDomElement u = acquisition.toElement();
                        if (!u.isNull()) {
                            tag = u.tagName();
                            if (tag == NB_CHANNELS) {
                                nbChannels = u.text().toInt();
                                return nbChannels;
                            }
                        }
                        acquisition = acquisition.nextSibling();
                    }
                    break;
                }
            }
            n = n.nextSibling();
        }
    }
    return nbChannels;
}

double NeuroscopeXmlReader::getSamplingRate()const{
    double samplingRate = 0;

    QDomNode n = documentNode.firstChild();
    if (!n.isNull()) {
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull()) {
                QString tag = e.tagName();
                if (tag == ACQUISITION) {
                    QDomNode acquisition = e.firstChild(); // try to convert the node to an element.
                    while(!acquisition.isNull()) {
                        QDomElement u = acquisition.toElement();
                        if (!u.isNull()) {
                            tag = u.tagName();
                            if (tag == SAMPLING_RATE) {
                                samplingRate = u.text().toDouble();
                                return samplingRate;
                            }
                        }
                        acquisition = acquisition.nextSibling();
                    }
                    break;
                }
            }
            n = n.nextSibling();
        }
    }
    return samplingRate;
}

double NeuroscopeXmlReader::getUpsamplingRate()const{
    double upsamplingRate = 0;

    QDomNode n = documentNode.firstChild();
    if (!n.isNull()) {
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull()) {
                QString tag = e.tagName();
                if (tag == NEUROSCOPE) {
                    QDomNode video = e.firstChildElement(SPIKES); // try to convert the node to an element.
                    if (!video.isNull()) {
                        QDomNode b = video.firstChild();
                        while(!b.isNull()) {
                            QDomElement w = b.toElement();
                            if(!w.isNull()) {
                                tag = w.tagName();
                                if (tag == UPSAMPLING_RATE) {
                                    upsamplingRate =  w.text().toDouble();
                                    return upsamplingRate;
                                }
                            }
                            b = b.nextSibling();
                        }
                    }
                }
            }
            n = n.nextSibling();
        }
    }
    return upsamplingRate;
}


double NeuroscopeXmlReader::getLfpInformation()const{
    double lfpSamplingRate = 0;
    QDomNode n = documentNode.firstChild();
    if (!n.isNull()) {
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull()) {
                QString tag = e.tagName();
                if (tag == FIELD_POTENTIALS) {
                    QDomNode video = e.firstChild(); // try to convert the node to an element.
                    while(!video.isNull()) {
                        QDomElement u = video.toElement();
                        if (!u.isNull()) {
                            tag = u.tagName();
                            if (tag == LFP_SAMPLING_RATE) {
                                lfpSamplingRate = u.text().toDouble();
                                return lfpSamplingRate;
                            }
                        }
                        video = video.nextSibling();
                    }
                    break;
                }
            }
            n = n.nextSibling();
        }
    }
    return lfpSamplingRate;
}


float NeuroscopeXmlReader::getScreenGain() const{
    float gain = 0;

    QDomNode n = documentNode.firstChild();
    if (!n.isNull()) {
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull()) {
                QString tag = e.tagName();
                if (tag == NEUROSCOPE) {
                    QDomNode video = e.firstChildElement(MISCELLANEOUS); // try to convert the node to an element.
                    if (!video.isNull()) {
                        QDomNode b = video.firstChild();
                        while(!b.isNull()) {
                            QDomElement w = b.toElement();
                            if(!w.isNull()) {
                                tag = w.tagName();
                                if (tag == SCREENGAIN) {
                                    gain = w.text().toFloat();
                                    return gain;
                                }
                            }
                            b = b.nextSibling();
                        }
                    }
                }
            }
            n = n.nextSibling();
        }
    }
    return gain;
}


int NeuroscopeXmlReader::getVoltageRange() const{
    int range = 0;
    xmlXPathObjectPtr result;
    xmlChar* searchPath;
    //The tag has change of location, it was inside the MISCELLANEOUS element, it is now inside the ACQUISITION element.
    if(type == SESSION && readVersion.isEmpty() )
        searchPath = xmlCharStrdup(QString("//" + MISCELLANEOUS + "/" + VOLTAGE_RANGE).toLatin1());
    else
        searchPath = xmlCharStrdup(QString("//" + ACQUISITION + "/" + VOLTAGE_RANGE).toLatin1());

    //Evaluate xpath expression
    result = xmlXPathEvalExpression(searchPath,xpathContex);
    if(result != NULL){
        xmlNodeSetPtr nodeset = result->nodesetval;
        if(!xmlXPathNodeSetIsEmpty(nodeset)){
            //Should be only one range element, so take the first one.
            xmlChar* sRange = xmlNodeListGetString(doc,nodeset->nodeTab[0]->children, 1);
            range = QString((char*)sRange).toInt();
            xmlFree(sRange);
        }
    }

    xmlFree(searchPath);
    xmlXPathFreeObject(result);
    return range;
}


int NeuroscopeXmlReader::getAmplification() const{
    int amplification = 0;
    xmlXPathObjectPtr result;
    xmlChar* searchPath;
    //The tag has change of location, it was inside the MISCELLANEOUS element, it is now inside the ACQUISITION element.
    if(type == SESSION && readVersion.isEmpty())
        searchPath = xmlCharStrdup(QString("//" + MISCELLANEOUS + "/" + AMPLIFICATION).toLatin1());
    else
        searchPath = xmlCharStrdup(QString("//" + ACQUISITION + "/" + AMPLIFICATION).toLatin1());

    //Evaluate xpath expression
    result = xmlXPathEvalExpression(searchPath,xpathContex);
    if(result != NULL){
        xmlNodeSetPtr nodeset = result->nodesetval;
        if(!xmlXPathNodeSetIsEmpty(nodeset)){
            //Should be only one amplification element, so take the first one.
            xmlChar* sAmplification = xmlNodeListGetString(doc,nodeset->nodeTab[0]->children, 1);
            amplification = QString((char*)sAmplification).toInt();
            xmlFree(sAmplification);
        }
    }

    xmlFree(searchPath);
    xmlXPathFreeObject(result);
    return amplification;
}


int NeuroscopeXmlReader::getOffset()const{
    int offset = 0;
    xmlXPathObjectPtr result;
    xmlChar* searchPath;
    //The tag has change of location, it was inside the MISCELLANEOUS element, it is now inside the ACQUISITION element.
    if(type == SESSION && readVersion.isEmpty())
        searchPath = xmlCharStrdup(QString("//" + MISCELLANEOUS + "/" + OFFSET).toLatin1());
    else
        searchPath = xmlCharStrdup(QString("//" + ACQUISITION + "/" + OFFSET).toLatin1());

    //Evaluate xpath expression
    result = xmlXPathEvalExpression(searchPath,xpathContex);
    if(result != NULL){
        xmlNodeSetPtr nodeset = result->nodesetval;
        if(!xmlXPathNodeSetIsEmpty(nodeset)){
            //Should be only one offset element, so take the first one.
            xmlChar* sOffset = xmlNodeListGetString(doc,nodeset->nodeTab[0]->children, 1);
            offset = QString((char*)sOffset).toInt();
            xmlFree(sOffset);
        }
    }

    xmlFree(searchPath);
    xmlXPathFreeObject(result);

    return offset;
}


QList<ChannelDescription> NeuroscopeXmlReader::getChannelDescription(){
    QList<ChannelDescription> list;

    xmlXPathObjectPtr result;
    xmlChar* searchPath = xmlCharStrdup(QString("//" + CHANNELS + "/" + CHANNEL_COLORS).toLatin1());

    //Evaluate xpath expression
    result = xmlXPathEvalExpression(searchPath,xpathContex);
    if(result != NULL){
        xmlNodeSetPtr nodeset = result->nodesetval;
        if(!xmlXPathNodeSetIsEmpty(nodeset)){
            //loop on all the CHANNEL_COLORS.
            int nbChannels = nodeset->nodeNr;
            for(int i = 0; i < nbChannels; ++i){
                ChannelDescription channelDescription;
                xmlNodePtr child;
                for(child = nodeset->nodeTab[i]->children;child != NULL;child = child->next){
                    //skip the carriage return (text node named text and containing /n)
                    if(child->type == XML_TEXT_NODE) continue;

                    if(QString((char*)child->name) == CHANNEL){
                        xmlChar* sId = xmlNodeListGetString(doc,child->children, 1);
                        int channelId = QString((char*)sId).toInt();
                        xmlFree(sId);
                        channelDescription.setId(channelId) ;
                    }
                    if(QString((char*)child->name) == COLOR){
                        xmlChar* sColor = xmlNodeListGetString(doc,child->children, 1);
                        QString color = QString((char*)sColor);
                        xmlFree(sColor);
                        channelDescription.setColor(color) ;
                    }
                    if(QString((char*)child->name) == ANATOMY_COLOR){
                        xmlChar* sColor = xmlNodeListGetString(doc,child->children, 1);
                        QString color = QString((char*)sColor);
                        xmlFree(sColor);
                        channelDescription.setGroupColor(color) ;
                    }
                    if(QString((char*)child->name) == SPIKE_COLOR){
                        xmlChar* sColor = xmlNodeListGetString(doc,child->children, 1);
                        QString color = QString((char*)sColor);
                        xmlFree(sColor);
                        channelDescription.setSpikeGroupColor(color) ;
                    }
                }
                list.append(channelDescription);
            }
        }
    }

    xmlFree(searchPath);
    xmlXPathFreeObject(result);
    return list;
}

void NeuroscopeXmlReader::getChannelDefaultOffset(QMap<int,int>& channelDefaultOffsets){
    QDomNode n = documentNode.firstChild();
    if (!n.isNull()) {
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull()) {
                QString tag = e.tagName();
                if (tag == NEUROSCOPE) {
                    QDomNode channels = e.firstChildElement(CHANNELS); // try to convert the node to an element.
                    if (!channels.isNull()) {
                        QDomNode channelColors = channels.firstChild();
                        int i = 0;
                        while(!channelColors.isNull()) {
                            QDomElement w = channelColors.toElement();
                            if(!w.isNull()) {
                                tag = w.tagName();
                                if (tag == CHANNEL_OFFSET) {
                                    QDomNode channelGroup = w.firstChild(); // try to convert the node to an element.
                                    int channelId = i;
                                    int offset = 0;

                                    while(!channelGroup.isNull()) {
                                        QDomElement val = channelGroup.toElement();
                                        if (!val.isNull()) {
                                            tag = val.tagName();
                                            if (tag == CHANNEL) {
                                                channelId = val.text().toInt();
                                            } else if(tag == DEFAULT_OFFSET) {
                                                offset = val.text().toInt();
                                            }
                                        }
                                        //the channels must be numbered continuously from 0.
                                        //if(channelId < nbChannels)
                                            channelDefaultOffsets.insert(channelId,offset);
                                       channelGroup =  channelGroup.nextSibling();
                                    }
                                }
                            }
                            channelColors = channelColors.nextSibling();
                            i++;
                        }
                    }
                }
            }
            n = n.nextSibling();
        }
    }
}

void NeuroscopeXmlReader::getSpikeDescription(int nbChannels,QMap<int,int>& spikeChannelsGroups,QMap<int, QList<int> >& spikeGroupsChannels){ 
    //Anatomical goups and spike groups share the trash group. the spikeChannelsGroups already contains the trash group, if any, set after retrieving the anatomical groups information.
    //At first, if a channel is not in the trash group it is put in the undefined group, the -1 (this correspond to no spike group).
    //Then reading for the file, the right information is set.
    QList<int> trashList;
    if(spikeGroupsChannels.contains(0)) trashList = spikeGroupsChannels[0];
    QList<int> spikeTrashList;
    for(int i = 0; i < nbChannels; ++i){
        if(!trashList.contains(i)){
            spikeTrashList.append(i);
            spikeChannelsGroups.insert(i,-1);
        }
        else spikeChannelsGroups.insert(i,0);
    }

    xmlXPathObjectPtr result;
    xmlChar* searchPath;
    //The tag has change of location, it was inside CHANNEL_GROUPS/GROUP tag, it is now inside CHANNEL_GROUPS/GROUP/CHANNELS.
    if(readVersion.isEmpty() || readVersion == "1.2.2")
        searchPath = xmlCharStrdup(QString("//" + SPIKE + "/" + CHANNEL_GROUPS + "/" + GROUP).toLatin1());
    else
        searchPath = xmlCharStrdup(QString("//" + SPIKE + "/" + CHANNEL_GROUPS + "/" + GROUP + "/" + CHANNELS).toLatin1());

    //Evaluate xpath expression
    result = xmlXPathEvalExpression(searchPath,xpathContex);
    if(result != NULL){
        xmlNodeSetPtr nodeset = result->nodesetval;
        if(!xmlXPathNodeSetIsEmpty(nodeset)){
            //loop on all the GROUP.
            int nbGroups = nodeset->nodeNr;
            for(int i = 0; i < nbGroups; ++i){
                QList<int> channelList;
                xmlNodePtr child;
                for(child = nodeset->nodeTab[i]->children;child != NULL;child = child->next){
                    //skip the carriage return (text node named text and containing /n)
                    if(child->type == XML_TEXT_NODE) continue;

                    if(QString((char*)child->name) == CHANNEL){
                        xmlChar* sId = xmlNodeListGetString(doc,child->children, 1);
                        int channelId = QString((char*)sId).toInt();
                        xmlFree(sId);
                        channelList.append(channelId);
                        spikeChannelsGroups.insert(channelId,i + 1);//overwrite the entry for the spike trash group (-1)
                        //remove the channel from the spike trash list as it is part of a group
                        spikeTrashList.removeAll(channelId);
                    }
                }
                spikeGroupsChannels.insert(i + 1,channelList);
            }
        }
    }

    if(spikeTrashList.size() != 0) spikeGroupsChannels.insert(-1,spikeTrashList);

    xmlFree(searchPath);
    xmlXPathFreeObject(result);
}

void NeuroscopeXmlReader::getAnatomicalDescription(int nbChannels,QMap<int,int>& displayChannelsGroups,QMap<int, QList<int> >& displayGroupsChannels,QMap<int,bool>& skipStatus){
    //First, everything is put in the trash group with a skip status at false (this correspond to no anatomical group).
    //Then reading for the file, the right information is set.
    QList<int> trashList;
    for(int i = 0; i < nbChannels; ++i){
        trashList.append(i);
        displayChannelsGroups.insert(i,0);
        skipStatus.insert(i,false);
    }

    xmlXPathObjectPtr result;
    xmlChar* searchPath = xmlCharStrdup(QString("//" + ANATOMY + "/" + CHANNEL_GROUPS + "/" + GROUP).toLatin1());

    //Evaluate xpath expression
    result = xmlXPathEvalExpression(searchPath,xpathContex);
    if(result != NULL){
        xmlNodeSetPtr nodeset = result->nodesetval;
        if(!xmlXPathNodeSetIsEmpty(nodeset)){
            //loop on all the GROUP.
            int nbGroups = nodeset->nodeNr;
            for(int i = 0; i < nbGroups; ++i){
                QList<int> channelList;
                xmlNodePtr child;
                for(child = nodeset->nodeTab[i]->children;child != NULL;child = child->next){
                    //skip the carriage return (text node named text and containing /n)
                    if(child->type == XML_TEXT_NODE) continue;

                    if(QString((char*)child->name) == CHANNEL){
                        xmlChar* sId = xmlNodeListGetString(doc,child->children, 1);
                        int channelId = QString((char*)sId).toInt();
                        xmlFree(sId);
                        channelList.append(channelId);
                        displayChannelsGroups.insert(channelId,i + 1);//overwrite the entry for the trash group (0)
                        //remove the channel from the trash list as it is part of a group
                        trashList.removeAll(channelId);

                        //Look up for the SKIP attribute
                        xmlChar* skipTag = xmlCharStrdup(QString(SKIP).toLatin1());
                        xmlChar* sSkip = xmlGetProp(child,skipTag);
                        if(sSkip != NULL){
                            skipStatus.insert(channelId,QString((char*)sSkip).toInt());
                        }
                        xmlFree(skipTag);
                        xmlFree(sSkip);
                    }
                }
                displayGroupsChannels.insert(i + 1,channelList);
            }
        }
    }

    if(trashList.size() != 0) displayGroupsChannels.insert(0,trashList);

    xmlFree(searchPath);
    xmlXPathFreeObject(result);
}

int NeuroscopeXmlReader::getNbSamples()const{
    int nbSamples = 0;
    xmlXPathObjectPtr result;
    xmlChar* searchPath;
    //The tag has change of location, it was inside the SPIKE element, it is now inside the NEUROSCOPE/SPIKES element.
    if(type == SESSION && (readVersion == "" || readVersion == "1.2.2"))
        searchPath = xmlCharStrdup(QString("//" + SPIKE + "/" + NB_SAMPLES).toLatin1());
    else
        searchPath = xmlCharStrdup(QString("//" + NEUROSCOPE + "/" + SPIKES + "/" + NB_SAMPLES).toLatin1());


    //Evaluate xpath expression
    result = xmlXPathEvalExpression(searchPath,xpathContex);
    if(result != NULL){
        xmlNodeSetPtr nodeset = result->nodesetval;
        if(!xmlXPathNodeSetIsEmpty(nodeset)){
            //Should be only one nbSamples element, so take the first one.
            xmlChar* sNbSamples = xmlNodeListGetString(doc,nodeset->nodeTab[0]->children, 1);
            nbSamples = QString((char*)sNbSamples).toInt();
            xmlFree(sNbSamples);
        }
    }

    xmlFree(searchPath);
    xmlXPathFreeObject(result);
    return nbSamples;
}

float NeuroscopeXmlReader::getWaveformLength()const{
    float waveformLength = 0;

    QDomNode n = documentNode.firstChild();
    if (!n.isNull()) {
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull()) {
                QString tag = e.tagName();
                if (tag == NEUROSCOPE) {
                    QDomNode video = e.firstChildElement(SPIKES); // try to convert the node to an element.
                    if (!video.isNull()) {
                        QDomNode b = video.firstChild();
                        while(!b.isNull()) {
                            QDomElement w = b.toElement();
                            if(!w.isNull()) {
                                tag = w.tagName();
                                if (tag == WAVEFORM_LENGTH) {
                                    waveformLength =  w.text().toFloat();
                                    return waveformLength;
                                }
                            }
                            b = b.nextSibling();
                        }
                    }
                }
            }
            n = n.nextSibling();
        }
    }
    return waveformLength;
}


int NeuroscopeXmlReader::getPeakSampleIndex()const{
    int index = 0;
    xmlXPathObjectPtr result;
    xmlChar* searchPath;
    //The tag has change of location, it was inside the SPIKE element, it is now inside the NEUROSCOPE/SPIKES element.
    if(type == SESSION && (readVersion == "" || readVersion == "1.2.2")) searchPath = xmlCharStrdup(QString("//" + SPIKE + "/" + PEAK_SAMPLE_INDEX).toLatin1());
    else searchPath = xmlCharStrdup(QString("//" + NEUROSCOPE + "/" + SPIKES + "/" + PEAK_SAMPLE_INDEX).toLatin1());


    //Evaluate xpath expression
    result = xmlXPathEvalExpression(searchPath,xpathContex);
    if(result != NULL){
        xmlNodeSetPtr nodeset = result->nodesetval;
        if(!xmlXPathNodeSetIsEmpty(nodeset)){
            //Should be only one index element, so take the first one.
            xmlChar* sindex = xmlNodeListGetString(doc,nodeset->nodeTab[0]->children, 1);
            index = QString((char*)sindex).toInt();
            xmlFree(sindex);
        }
    }

    xmlFree(searchPath);
    xmlXPathFreeObject(result);
    return index;
}

float NeuroscopeXmlReader::getPeakSampleLength()const{
    float indexLength = 0;


    QDomNode n = documentNode.firstChild();
    if (!n.isNull()) {
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull()) {
                QString tag = e.tagName();
                if (tag == NEUROSCOPE) {
                    QDomNode video = e.firstChildElement(SPIKES); // try to convert the node to an element.
                    if (!video.isNull()) {
                        QDomNode b = video.firstChild();
                        while(!b.isNull()) {
                            QDomElement w = b.toElement();
                            if(!w.isNull()) {
                                tag = w.tagName();
                                if (tag == PEAK_SAMPLE_LENGTH) {
                                    indexLength =  w.text().toFloat();
                                    return indexLength;
                                }
                            }
                            b = b.nextSibling();
                        }
                    }
                }
            }
            n = n.nextSibling();
        }
    }

    return indexLength;
}


int NeuroscopeXmlReader::getVideoWidth()const{
    int width = 0;
    QDomNode n = documentNode.firstChild();
    if (!n.isNull()) {
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull()) {
                QString tag = e.tagName();
                if (tag == VIDEO) {
                    QDomNode video = e.firstChild(); // try to convert the node to an element.
                    while(!video.isNull()) {
                        QDomElement u = video.toElement();
                        if (!u.isNull()) {
                            tag = u.tagName();
                            if(tag == WIDTH) {
                                width = u.text().toInt();
                                return width;
                            }
                        }
                        video = video.nextSibling();
                    }
                    break;
                }
            }
            n = n.nextSibling();
        }
    }
    return width;
}


int NeuroscopeXmlReader::getVideoHeight()const{
    int height = 0;
    QDomNode n = documentNode.firstChild();
    if (!n.isNull()) {
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull()) {
                QString tag = e.tagName();
                if (tag == VIDEO) {
                    QDomNode video = e.firstChild(); // try to convert the node to an element.
                    while(!video.isNull()) {
                        QDomElement u = video.toElement();
                        if (!u.isNull()) {
                            tag = u.tagName();
                            if(tag == HEIGHT) {
                                height = u.text().toInt();
                                return height;
                            }
                        }
                        video = video.nextSibling();
                    }
                    break;
                }
            }
            n = n.nextSibling();
        }
    }
    return height;
}


int NeuroscopeXmlReader::getRotation()const{
    int angle = 0;

    QDomNode n = documentNode.firstChild();
    if (!n.isNull()) {
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull()) {
                QString tag = e.tagName();
                if (tag == VIDEO) {
                    QDomNode video = e.firstChild(); // try to convert the node to an element.
                    while(!video.isNull()) {
                        QDomElement u = video.toElement();
                        if (!u.isNull()) {
                            tag = u.tagName();
                            if(tag == ROTATE) {
                                angle = u.text().toInt();
                                return angle;
                            }
                        }
                        video = video.nextSibling();
                    }
                    break;
                }
            }
            n = n.nextSibling();
        }
    }
    return angle;
}


int NeuroscopeXmlReader::getFlip()const{
    int orientation = 0;
    QDomNode n = documentNode.firstChild();
    if (!n.isNull()) {
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull()) {
                QString tag = e.tagName();
                if (tag == VIDEO) {
                    QDomNode video = e.firstChild(); // try to convert the node to an element.
                    while(!video.isNull()) {
                        QDomElement u = video.toElement();
                        if (!u.isNull()) {
                            tag = u.tagName();
                            if(tag == FLIP) {
                                orientation = u.text().toInt();
                                return orientation;
                            }
                        }
                        video = video.nextSibling();
                    }
                    break;
                }
            }
            n = n.nextSibling();
        }
    }
    return orientation;
}

int NeuroscopeXmlReader::getTrajectory()const{
    int drawTrajectory = 0;
    QDomNode n = documentNode.firstChild();
    if (!n.isNull()) {
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull()) {
                QString tag = e.tagName();
                if (tag == VIDEO) {
                    QDomNode video = e.firstChild(); // try to convert the node to an element.
                    while(!video.isNull()) {
                        QDomElement u = video.toElement();
                        if (!u.isNull()) {
                            tag = u.tagName();
                            if(tag == POSITIONS_BACKGROUND) {
                                drawTrajectory = u.text().toInt();
                                return drawTrajectory;
                            }
                        }
                        video = video.nextSibling();
                    }
                    break;
                }
            }
            n = n.nextSibling();
        }
    }
    return drawTrajectory;
}  

QString NeuroscopeXmlReader::getBackgroundImage()const{
    QString backgroundPath = "-";
    QDomNode n = documentNode.firstChild();
    if (!n.isNull()) {
        while(!n.isNull()) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if(!e.isNull()) {
                QString tag = e.tagName();
                if (tag == VIDEO) {
                    QDomNode video = e.firstChild(); // try to convert the node to an element.
                    while(!video.isNull()) {
                        QDomElement u = video.toElement();
                        if (!u.isNull()) {
                            tag = u.tagName();
                            if(tag == VIDEO_IMAGE) {
                                backgroundPath = u.text();
                                return backgroundPath;
                            }
                        }
                        video = video.nextSibling();
                    }
                    break;
                }
            }
            n = n.nextSibling();
        }
    }
    return backgroundPath;
}  

QString NeuroscopeXmlReader::getTraceBackgroundImage()const{
    QString traceBackgroundPath = "-";
    xmlXPathObjectPtr result;
    xmlChar* searchPath = xmlCharStrdup(QString("//" + MISCELLANEOUS + "/" + TRACE_BACKGROUND_IMAGE).toLatin1());

    //Evaluate xpath expression
    result = xmlXPathEvalExpression(searchPath,xpathContex);
    if(result != NULL){
        xmlNodeSetPtr nodeset = result->nodesetval;
        if(!xmlXPathNodeSetIsEmpty(nodeset)){
            //Should be only one TRACE_BACKGROUND_IMAGE element, so take the first one.
            xmlChar* sTraceBackgroundPath = xmlNodeListGetString(doc,nodeset->nodeTab[0]->children, 1);
            traceBackgroundPath = QString((char*)sTraceBackgroundPath);
            xmlFree(sTraceBackgroundPath);
        }
    }

    xmlFree(searchPath);
    xmlXPathFreeObject(result);

    return traceBackgroundPath;
} 

QList<SessionFile> NeuroscopeXmlReader::getFilesToLoad(){
    QList<SessionFile> list;
    xmlXPathObjectPtr result;
    xmlChar* searchPath = xmlCharStrdup(QString("/" + NEUROSCOPE + "/" + FILES + "/" + neuroscope::FILE).toLatin1());

    //Evaluate xpath expression
    result = xmlXPathEvalExpression(searchPath,xpathContex);
    if(result != NULL){
        xmlNodeSetPtr nodeset = result->nodesetval;
        if(!xmlXPathNodeSetIsEmpty(nodeset)){
            //loop on all the FILE.
            int nbFiles = nodeset->nodeNr;
            for(int i = 0; i < nbFiles; ++i){
                SessionFile sessionFile;
                xmlNodePtr child;
                for(child = nodeset->nodeTab[i]->children;child != NULL;child = child->next){
                    //skip the carriage return (text node named text and containing /n)
                    if(child->type == XML_TEXT_NODE) continue;

                    if(QString((char*)child->name) == TYPE){
                        xmlChar* sType = xmlNodeListGetString(doc,child->children, 1);
                        int type = QString((char*)sType).toInt();
                        xmlFree(sType);
                        sessionFile.setType(static_cast<SessionFile::type>(type)) ;
                    }
                    if(QString((char*)child->name) == URL){
                        xmlChar* sUrl = xmlNodeListGetString(doc,child->children, 1);
                        QString url = QString((char*)sUrl);
                        xmlFree(sUrl);
                        sessionFile.setUrl(QString(url));
                    }
                    if(QString((char*)child->name) == DATE){
                        xmlChar* sDate = xmlNodeListGetString(doc,child->children, 1);
                        QString date = QString((char*)sDate);
                        xmlFree(sDate);
                        sessionFile.setModification(QDateTime::fromString(date,Qt::ISODate));
                    }
                    if(QString((char*)child->name) == ITEMS){
                        //loop on the items
                        xmlNodePtr items;
                        for(items = child->children;items != NULL;items = items->next){
                            //skip the carriage return (text node named text and containing /n)
                            if(items->type == XML_TEXT_NODE) continue;

                            //loop on the elements in each itemDescription tag (item and color)
                            xmlNodePtr itemDescription;
                            QString id;
                            QString color;
                            for(itemDescription = items->children;itemDescription != NULL;itemDescription = itemDescription->next){
                                //skip the carriage return (text node named text and containing /n)
                                if(itemDescription->type == XML_TEXT_NODE) continue;

                                if(QString((char*)itemDescription->name) == ITEM){
                                    xmlChar* sId = xmlNodeListGetString(doc,itemDescription->children, 1);
                                    id = QString((char*)sId);
                                    xmlFree(sId);
                                }
                                if(QString((char*)itemDescription->name) == COLOR){
                                    xmlChar* sColor = xmlNodeListGetString(doc,itemDescription->children, 1);
                                    color = QString((char*)sColor);
                                    xmlFree(sColor);
                                }
                            }
                            sessionFile.setItemColor(id,color);
                        }
                    }
                    //obsolet, here for backware compatibility
                    if(QString((char*)child->name) == VIDEO_IMAGE){
                        xmlChar* sBackgroundPath = xmlNodeListGetString(doc,child->children, 1);
                        QString backgroundPath = QString((char*)sBackgroundPath);
                        xmlFree(sBackgroundPath);
                        sessionFile.setBackgroundPath(backgroundPath);
                    }
                }
                list.append(sessionFile);
            }
        }
    }

    xmlFree(searchPath);
    xmlXPathFreeObject(result);
    return list;

}


QList<DisplayInformation> NeuroscopeXmlReader::getDisplayInformation(){
    QList<DisplayInformation> list;
    xmlXPathObjectPtr result;
    xmlChar* searchPath = xmlCharStrdup(QString("/" + NEUROSCOPE + "/" + DISPLAYS + "/" + DISPLAY).toLatin1());

    //Evaluate xpath expression
    result = xmlXPathEvalExpression(searchPath,xpathContex);
    if(result != NULL){

        xmlNodeSetPtr nodeset = result->nodesetval;
        if(!xmlXPathNodeSetIsEmpty(nodeset)){
            //loop on all the DISPLAY.
            int nbDisplays = nodeset->nodeNr;

            for(int i = 0; i < nbDisplays; ++i){
                DisplayInformation displayInformation;
                xmlNodePtr child;
                for(child = nodeset->nodeTab[i]->children;child != NULL;child = child->next){
                    //skip the carriage return (text node named text and containing /n)
                    if(child->type == XML_TEXT_NODE) continue;

                    if(QString((char*)child->name) == TAB_LABEL){
                        xmlChar* sLabel = xmlNodeListGetString(doc,child->children, 1);
                        QString label = QString((char*)sLabel);
                        xmlFree(sLabel);
                        displayInformation.setTabLabel(label);
                    }

                    if(QString((char*)child->name) == SHOW_LABELS){
                        xmlChar* sShowLabels = xmlNodeListGetString(doc,child->children, 1);
                        int showLabels = QString((char*)sShowLabels).toInt();
                        xmlFree(sShowLabels);
                        displayInformation.setLabelStatus(showLabels);
                    }

                    if(QString((char*)child->name) == START_TIME){
                        xmlChar* sStartTime = xmlNodeListGetString(doc,child->children, 1);
                        long startTime = QString((char*)sStartTime).toLong();
                        xmlFree(sStartTime);
                        displayInformation.setStartTime(startTime);
                    }

                    if(QString((char*)child->name) == DURATION){
                        xmlChar* sDuration = xmlNodeListGetString(doc,child->children, 1);
                        long duration = QString((char*)sDuration).toLong();
                        xmlFree(sDuration);
                        displayInformation.setTimeWindow(duration);
                    }

                    if(QString((char*)child->name) == MULTIPLE_COLUMNS){
                        xmlChar* sPresentation = xmlNodeListGetString(doc,child->children, 1);
                        int presentationMode = QString((char*)sPresentation).toInt();
                        xmlFree(sPresentation);
                        displayInformation.setMode(static_cast<DisplayInformation::mode>(presentationMode));
                    }

                    if(QString((char*)child->name) == GREYSCALE){
                        xmlChar* sGreyScale = xmlNodeListGetString(doc,child->children, 1);
                        int greyScale = QString((char*)sGreyScale).toInt();
                        xmlFree(sGreyScale);
                        displayInformation.setGreyScale(greyScale);
                    }

                    if(QString((char*)child->name) == POSITIONVIEW){
                        xmlChar* sPositionView = xmlNodeListGetString(doc,child->children, 1);
                        int positionView = QString((char*)sPositionView).toInt();
                        xmlFree(sPositionView);
                        displayInformation.setPositionView(positionView);
                    }

                    if(QString((char*)child->name) == SHOWEVENTS){
                        xmlChar* sShowEvents = xmlNodeListGetString(doc,child->children, 1);
                        int showEvents = QString((char*)sShowEvents).toInt();
                        xmlFree(sShowEvents);
                        displayInformation.setEventsInPositionView(showEvents);
                    }

                    if(QString((char*)child->name) == SPIKE_PRESENTATION){
                        xmlChar* sPresentation = xmlNodeListGetString(doc,child->children, 1);
                        int spikePresentation = QString((char*)sPresentation).toInt();
                        xmlFree(sPresentation);
                        displayInformation.addSpikeDisplayType(static_cast<DisplayInformation::spikeDisplayType>(spikePresentation));
                    }

                    if(QString((char*)child->name) == RASTER_HEIGHT){
                        xmlChar* sheight = xmlNodeListGetString(doc,child->children, 1);
                        int height = QString((char*)sheight).toInt();
                        xmlFree(sheight);
                        displayInformation.setRasterHeight(height);
                    }

                    if(QString((char*)child->name) == CLUSTERS_SELECTED){
                        //loop on the CLUSTERS
                        xmlNodePtr clusters;
                        QString clusterFile;
                        QList<int> clusterIds;
                        for(clusters = child->children;clusters != NULL;clusters = clusters->next){
                            //skip the carriage return (text node named text and containing /n)
                            if(clusters->type == XML_TEXT_NODE) continue;

                            if(QString((char*)clusters->name) == FILE_URL){
                                xmlChar* sUrl = xmlNodeListGetString(doc,clusters->children, 1);
                                clusterFile = QString((char*)sUrl);
                                xmlFree(sUrl);
                            }
                            if(QString((char*)clusters->name) == CLUSTER){
                                xmlChar* sClusterId = xmlNodeListGetString(doc,clusters->children, 1);
                                int clusterId = QString((char*)sClusterId).toInt();
                                xmlFree(sClusterId);
                                clusterIds.append(clusterId);
                            }
                        }
                        displayInformation.setSelectedClusters(clusterFile,clusterIds);
                    }

                    if(QString((char*)child->name) == SPIKES_SELECTED){
                        QStringList files;
                        //loop on the urls of the files
                        xmlNodePtr spikes;
                        for(spikes = child->children;spikes != NULL;spikes = spikes->next){
                            //skip the carriage return (text node named text and containing /n)
                            if(spikes->type == XML_TEXT_NODE) continue;

                            if(QString((char*)spikes->name) == FILE_URL){
                                xmlChar* sUrl = xmlNodeListGetString(doc,spikes->children, 1);
                                files.append(QString((char*)sUrl));
                                xmlFree(sUrl);
                            }
                        }
                        displayInformation.setSelectedSpikeFiles(files);
                    }

                    if(QString((char*)child->name) == EVENTS_SELECTED){
                        //loop on the EVENTS
                        xmlNodePtr events;
                        QString eventFile;
                        QList<int> eventIds;
                        for(events = child->children;events != NULL;events = events->next){
                            //skip the carriage return (text node named text and containing /n)
                            if(events->type == XML_TEXT_NODE) continue;

                            if(QString((char*)events->name) == FILE_URL){
                                xmlChar* sUrl = xmlNodeListGetString(doc,events->children, 1);
                                eventFile = QString((char*)sUrl);
                                xmlFree(sUrl);
                            }
                            if(QString((char*)events->name) == EVENT){
                                xmlChar* sEventId = xmlNodeListGetString(doc,events->children, 1);
                                int eventId = QString((char*)sEventId).toInt();
                                xmlFree(sEventId);
                                eventIds.append(eventId);
                            }
                        }
                        displayInformation.setSelectedEvents(eventFile,eventIds);
                    }

                    if(QString((char*)child->name) == CLUSTERS_SKIPPED){
                        //loop on the CLUSTERS
                        xmlNodePtr clusters;
                        QString clusterFile;
                        QList<int> clusterIds;
                        for(clusters = child->children;clusters != NULL;clusters = clusters->next){
                            //skip the carriage return (text node named text and containing /n)
                            if(clusters->type == XML_TEXT_NODE) continue;

                            if(QString((char*)clusters->name) == FILE_URL){
                                xmlChar* sUrl = xmlNodeListGetString(doc,clusters->children, 1);
                                clusterFile = QString((char*)sUrl);
                                xmlFree(sUrl);
                            }
                            if(QString((char*)clusters->name) == CLUSTER){
                                xmlChar* sClusterId = xmlNodeListGetString(doc,clusters->children, 1);
                                int clusterId = QString((char*)sClusterId).toInt();
                                xmlFree(sClusterId);
                                clusterIds.append(clusterId);
                            }
                        }
                        displayInformation.setSkippedClusters(clusterFile,clusterIds);
                    }

                    if(QString((char*)child->name) == EVENTS_SKIPPED){
                        //loop on the EVENTS
                        xmlNodePtr events;
                        QString eventFile;
                        QList<int> eventIds;
                        for(events = child->children;events != NULL;events = events->next){
                            //skip the carriage return (text node named text and containing /n)
                            if(events->type == XML_TEXT_NODE) continue;

                            if(QString((char*)events->name) == FILE_URL){
                                xmlChar* sUrl = xmlNodeListGetString(doc,events->children, 1);
                                eventFile = QString((char*)sUrl);
                                xmlFree(sUrl);
                            }
                            if(QString((char*)events->name) == EVENT){
                                xmlChar* sEventId = xmlNodeListGetString(doc,events->children, 1);
                                int eventId = QString((char*)sEventId).toInt();
                                xmlFree(sEventId);
                                eventIds.append(eventId);
                            }
                        }
                        displayInformation.setSkippedEvents(eventFile,eventIds);
                    }


                    if(QString((char*)child->name) == CHANNEL_POSITIONS){
                        QList<TracePosition> positions;
                        //loop on the POSITIONS
                        xmlNodePtr positionElements;
                        for(positionElements = child->children;positionElements != NULL;positionElements = positionElements->next){
                            //skip the carriage return (text node named text and containing /n)
                            if(positionElements->type == XML_TEXT_NODE) continue;

                            TracePosition tracePosition;
                            //loop on the elements in each positions tag (channelId, gain and offset)
                            xmlNodePtr positionInfo;
                            for(positionInfo = positionElements->children;positionInfo != NULL;positionInfo = positionInfo->next){
                                //skip the carriage return (text node named text and containing /n)
                                if(positionInfo->type == XML_TEXT_NODE) continue;

                                if(QString((char*)positionInfo->name) == CHANNEL){
                                    xmlChar* sId = xmlNodeListGetString(doc,positionInfo->children, 1);
                                    int channelId = QString((char*)sId).toInt();
                                    xmlFree(sId);
                                    tracePosition.setId(channelId) ;
                                }
                                if(QString((char*)positionInfo->name) == GAIN){
                                    xmlChar* sGain = xmlNodeListGetString(doc,positionInfo->children, 1);
                                    int gain = QString((char*)sGain).toInt();
                                    tracePosition.setGain(gain);
                                    xmlFree(sGain);
                                }
                                if(QString((char*)positionInfo->name) == OFFSET){
                                    xmlChar* sOffset = xmlNodeListGetString(doc,positionInfo->children, 1);
                                    int offset = QString((char*)sOffset).toInt();
                                    tracePosition.setOffset(offset);
                                    xmlFree(sOffset);
                                }
                            }
                            positions.append(tracePosition);
                        }

                        displayInformation.setPositions(positions);
                    }

                    if(QString((char*)child->name) == CHANNELS_SELECTED){
                        QList<int> channelIds;
                        //loop on the urls of the files
                        xmlNodePtr channels;
                        for(channels = child->children;channels != NULL;channels = channels->next){
                            //skip the carriage return (text node named text and containing /n)
                            if(channels->type == XML_TEXT_NODE) continue;

                            if(QString((char*)channels->name) == CHANNEL){
                                xmlChar* sId = xmlNodeListGetString(doc,channels->children, 1);
                                int channelId = QString((char*)sId).toInt();
                                xmlFree(sId);
                                channelIds.append(channelId) ;
                            }
                        }
                        displayInformation.setSelectedChannelIds(channelIds);
                    }

                    if(QString((char*)child->name) == CHANNELS_SHOWN){
                        QList<int> channelIds;
                        //loop on the urls of the files
                        xmlNodePtr channels;
                        for(channels = child->children;channels != NULL;channels = channels->next){
                            //skip the carriage return (text node named text and containing /n)
                            if(channels->type == XML_TEXT_NODE) continue;

                            if(QString((char*)channels->name) == CHANNEL){
                                xmlChar* sId = xmlNodeListGetString(doc,channels->children, 1);
                                int channelId = QString((char*)sId).toInt();
                                xmlFree(sId);
                                channelIds.append(channelId) ;
                            }
                        }
                        displayInformation.setChannelIds(channelIds);
                    }

                }
                list.append(displayInformation);
            }
        }
    }

    xmlFree(searchPath);
    xmlXPathFreeObject(result);
    return list;
}


QMap<QString,double> NeuroscopeXmlReader::getSampleRateByExtension(){
    QMap<QString,double> samplingRatesMap;
    xmlXPathObjectPtr result;
    xmlChar* searchPath;

    //The tag has change of location, it was inside the session file (at SAMPLING_RATES/EXTENSION_SAMPLING_RATE),
    //it is now inside the the parameter file (at FILES/FILE).
    if(type == SESSION && (readVersion.isEmpty() || readVersion == "1.2.2"))
        searchPath = xmlCharStrdup(QString("/" + NEUROSCOPE + "/" + SAMPLING_RATES + "/" + EXTENSION_SAMPLING_RATE).toLatin1());
    else if(type == PARAMETER)
        searchPath = xmlCharStrdup(QString("//" + FILES + "/" + neuroscope::FILE).toLatin1());

    //Evaluate xpath expression
    result = xmlXPathEvalExpression(searchPath,xpathContex);
    if(result != NULL){
        xmlNodeSetPtr nodeset = result->nodesetval;
        if(!xmlXPathNodeSetIsEmpty(nodeset)){
            //loop on all the child.
            int nbExtensions = nodeset->nodeNr;
            for(int i = 0; i < nbExtensions; ++i){
                xmlNodePtr child;
                double samplingRate;
                QString extension;
                for(child = nodeset->nodeTab[i]->children;child != NULL;child = child->next){
                    //skip the carriage return (text node named text and containing /n)
                    if(child->type == XML_TEXT_NODE) continue;

                    if(QString((char*)child->name) == EXTENSION){
                        xmlChar* sExtendion = xmlNodeListGetString(doc,child->children, 1);
                        extension = QString((char*)sExtendion);
                        xmlFree(sExtendion);
                    }
                    if(QString((char*)child->name) == SAMPLING_RATE){
                        xmlChar* sSamplingRate = xmlNodeListGetString(doc,child->children, 1);
                        samplingRate = QString((char*)sSamplingRate).toDouble();
                        xmlFree(sSamplingRate);
                    }
                }
                samplingRatesMap.insert(extension,samplingRate);
            }
        }
    }

    xmlFree(searchPath);
    xmlXPathFreeObject(result);
    return samplingRatesMap;
}



