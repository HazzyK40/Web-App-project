

#include "GPXParser.h"
#include "LinkedListAPI.h"
#include <math.h>


GPXData* constructData(xmlNode * a_node);
Waypoint* constructWPT(xmlNode * a_node);
Route* constructRoute(xmlNode * a_node);
Track* constructTRACK(xmlNode * a_node);
void freeString(char* s);
char *concat(const char *s1, const char *s2);
int validateGPXfile(char *XMLFileName, char *XSDFileName);
int writeDoc(GPXdoc* gpxdoc, char *file);
void builtWayPoint(xmlNodePtr parent_node, Waypoint* nd);
bool validateWaypoint(Waypoint* nd);
bool validateGPXData(GPXData* dt);
char* gpxDataToJson(GPXData* data);
char* wayPointToJson(Waypoint* wp);
float calcDistance(double latHome, double lonHome, double latDest, double lonDest);
Waypoint* constructWPT(xmlNode * a_node){
    Waypoint* wNode = malloc(sizeof(Waypoint));
    wNode->name = "";
    wNode->otherData = initializeList(gpxDataToString, deleteGpxData, compareGpxData);
    struct _xmlAttr* attr = a_node->properties;
    while(attr!=NULL){
        if(strcmp((char *)attr->name, "lat")==0){
            char *ptr;
            wNode->latitude = strtod((char *)attr->children->content, &ptr);
        }
        if(strcmp((char *)attr->name, "lon")==0){
            char *ptr;
            wNode->longitude = strtod((char *)attr->children->content, &ptr);
        }
        attr = attr->next;
    }
    xmlNode* child = a_node->children;
    while(child!=NULL){
        if(strcmp((char *)child->name,"text")!=0){
            if(strcmp((char *)child->name,"name")==0){
                wNode->name =  concat(wNode->name, (char *) child->children->content);
            }else{
                insertBack(wNode->otherData, constructData(child));
            }
        }
        child = child->next;
    }
    return wNode;
}

Route* constructRoute(xmlNode * a_node){
    Route* rNode = malloc(sizeof(Route));
    rNode->name = "";
    rNode->otherData = initializeList(gpxDataToString, deleteGpxData, compareGpxData);
    rNode->waypoints = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
    xmlNode* child = a_node->children;
    while(child!=NULL){
        if(strcmp((char *)child->name, "rtept")==0){
            insertBack(rNode->waypoints, constructWPT(child));
        }else{
            if(strcmp((char *)child->name,"text")!=0){
                if(strcmp((char *)child->name,"name")==0){
                    rNode->name = concat(rNode->name, (char *) child->children->content);
                }else{
                    insertBack(rNode->otherData, constructData(child));
                }
            }
        }
        child=child->next;
    }
    return rNode;
}

Track* constructTRACK(xmlNode * a_node){
    Track* trkNode = malloc(sizeof(Track));
    trkNode->name = "";
    trkNode->otherData = initializeList(gpxDataToString, deleteGpxData, compareGpxData);
    trkNode->segments = initializeList(trackSegmentToString,deleteTrackSegment, compareTrackSegments);
    xmlNode* child = a_node->children;
    while(child!=NULL){
        if(strcmp((char *)child->name,"trkseg")==0){
            TrackSegment* seg = malloc(sizeof(TrackSegment));
            seg->waypoints = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
            xmlNode* a_node_c = child->children;
            while(a_node_c!=NULL){
                if(strcmp((char *)a_node_c->name,"trkpt")==0){
                    insertBack(seg->waypoints, constructWPT(a_node_c));
                }
                a_node_c = a_node_c->next;
            }
            insertBack(trkNode->segments, seg);
        }else{
            if(strcmp((char *)child->name,"text")!=0){
                if(strcmp((char *)child->name,"name")==0){
                    trkNode->name = concat(trkNode->name, (char *) child->children->content);
                }else{
                    insertBack(trkNode->otherData, constructData(child));
                }
            }
        }
        child = child->next;
    }
    return trkNode;
}

GPXData* constructData(xmlNode * a_node){
    char* val = (char *)a_node->children->content;
    GPXData* data = malloc(sizeof(GPXData)+strlen(val)+50);
    strcpy(data->name, (char *) a_node->name);
    strcpy(data->value, val);
    return data;
}

char *concat(const char *s1, const char *s2) {
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    char *p = (char *)malloc(len1 + len2 + 1);
    if (p) {
        memcpy(p, s1, len1);
        memcpy(p + len1, s2, len2 + 1);
    }
    return p;
}

//Main Methods A1 Module 1

GPXdoc* createGPXdoc(char* fileName){
    xmlDocPtr doc;
    xmlNode *a_node = NULL;
    doc = xmlReadFile(fileName, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse %s\n", fileName);
        return NULL;
    }
    a_node = xmlDocGetRootElement(doc);
    struct _xmlAttr* attr = a_node->properties;
    GPXdoc* gpxDoc = malloc(sizeof(GPXdoc));
    while(attr!=NULL){
        if(strcmp((char *)attr->name, "version")==0){
            char *ptr;
            gpxDoc->version = strtod((char *)attr->children->content, &ptr);
        }
        if(strcmp((char *)attr->name, "creator")==0){
            gpxDoc->creator = "";
            gpxDoc->creator = concat(gpxDoc->creator, (char *)attr->children->content);
        }
        if(strcmp((char *)attr->name, "schemaLocation")==0){
            strcpy(gpxDoc->namespace, (char *)attr->children->content);
        }
        attr = attr->next;
    }
    
    gpxDoc->waypoints = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
    gpxDoc->tracks = initializeList(trackToString, deleteTrack, compareTracks);
    gpxDoc->routes= initializeList(routeToString, deleteRoute, compareRoutes);
    xmlNode *cur_node = NULL;
    for (cur_node = a_node->children; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            char* name = (char *)cur_node->name;
            if(strcmp(name, "rte")==0){
                insertBack(gpxDoc->routes, constructRoute(cur_node));
            }else if(strcmp(name, "wpt")==0){
                Waypoint* n = constructWPT(cur_node);
                insertBack(gpxDoc->waypoints, n);
            }else if(strcmp(name, "trk")==0){
                insertBack(gpxDoc->tracks, constructTRACK(cur_node));
            }
        }
    }
    return gpxDoc;
}

char* GPXdocToString(GPXdoc* doc){
    char* str = "";
    str = concat(str, "\nGPXDoc");
    str = concat(str, "\nNamespace: ");
    str = concat(str, doc->namespace);
    
    str = concat(str, "\nCreater: ");
    str = concat(str, doc->creator);
    
    char v[4];
    gcvt(doc->version, 3, v);
    str = concat(str, "\nVersion: ");
    str = concat(str, v);
    
    ListIterator it = createIterator(doc->waypoints);
    ListIterator it2 = createIterator(doc->routes);
    ListIterator it3 = createIterator(doc->tracks);
    Waypoint* waypt;
    Route* rtep;
    Track* trkp;
    char* s = "";
    s = concat(s, "\n\nWayPoints: ");
    while((waypt = (Waypoint*) nextElement(&it)) != NULL){
        s = concat(s, waypointToString(waypt));
    }
    str = concat(str, s);
    char* s2 = "";
    s2 = concat(s2, "\n\nRoutes: ");
    while((rtep = (Route*) nextElement(&it2)) != NULL){
        s2 = concat(s2, routeToString(rtep));
    }
    str = concat(str, s2);
    char* s3 = "";
    s3 = concat(s3, "\n\nTracks: ");
    while((trkp = (Track*) nextElement(&it3)) != NULL){
        s3 = concat(s3, trackToString(trkp));
    }
    str = concat(str, s3);
    
    
    return str;
}


void deleteGPXdoc(GPXdoc* doc){
    free(doc->creator);
    clearList(doc->tracks);
    clearList(doc->routes);
    clearList(doc->waypoints);
    free(doc);
}



//Total number of waypoints in the GPX file
int getNumWaypoints(const GPXdoc* doc){
    if(doc==NULL){
        return 0;
    }
    int count = 0;
    count+=doc->waypoints->length;
    return count;
}

int getNumRoutes(const GPXdoc* doc){
    if(doc==NULL){
        return 0;
    }
    return doc->routes->length;
}

int getNumTracks(const GPXdoc* doc){
    if(doc==NULL){
        return 0;
    }
    return doc->tracks->length;
}

int getNumSegments(const GPXdoc* doc){
    if(doc==NULL){
        return 0;
    }
    int count = 0;
    ListIterator itr = createIterator(doc->tracks);
    Track* trks;
    while((trks = (Track*)nextElement(&itr))!=NULL){
        count+=trks->segments->length;
    }
    return count;
}

int getNumGPXData(const GPXdoc* doc){
    if(doc==NULL){
        return 0;
    }
    int count = 0;
    ListIterator itr1 = createIterator(doc->waypoints);
    Waypoint* wp;
    while((wp=(Waypoint*)nextElement(&itr1))!=NULL){
        if(strlen(wp->name)>0){
            count++;
        }
        count+=wp->otherData->length;
    }
    itr1 = createIterator(doc->routes);
    Route* routes;
    while((routes=(Route*)nextElement(&itr1))!=NULL){
        if(strlen(routes->name)>0){
            count++;
        }
        count+=routes->otherData->length;
        ListIterator itr2 = createIterator(routes->waypoints);
        while((wp=(Waypoint*)nextElement(&itr2))!=NULL){
            if(strlen(wp->name)>0){
                count++;
            }
            count+=wp->otherData->length;
        }
    }
    itr1 = createIterator(doc->tracks);
    Track* trk;
    while((trk=(Track*)nextElement(&itr1))!=NULL){
        count+=trk->otherData->length;
        if(strlen(trk->name)>0){
            count++;
        }
        ListIterator itr2 = createIterator(trk->segments);
        TrackSegment* seg;
        while((seg=(TrackSegment*)nextElement(&itr2))!=NULL){
            ListIterator itr3 = createIterator(seg->waypoints);
            while((wp=(Waypoint*)nextElement(&itr3))!=NULL){
                if(strlen(wp->name)>0){
                    count++;
                }
                count+=wp->otherData->length;
            }
        }
    }
    return count;
}


Waypoint* getWaypoint(const GPXdoc* doc, char* name){
    Waypoint* result;
    ListIterator it = createIterator(doc->waypoints);
    while((result=(Waypoint*)nextElement(&it))!=NULL){
        if(strcmp(result->name,name)==0){
            return result;
        }
    }
    return NULL;
}

Track* getTrack(const GPXdoc* doc, char* name){
    Track* trk;
    ListIterator it = createIterator(doc->tracks);
    while((trk=(Track*)nextElement(&it))!=NULL){
        if(strcmp(trk->name,name)==0){
            return trk;
        }
    }
    return NULL;
}

Route* getRoute(const GPXdoc* doc, char* name){
    Route* rte;
    ListIterator it = createIterator(doc->routes);
    while((rte=(Route*)nextElement(&it))!=NULL){
        if(strcmp(rte->name,name)==0){
            return rte;
        }
    }
    return NULL;
}




// helper functions

char* gpxDataToString(void *toBePrinted){
   GPXData* data;

   if (toBePrinted == NULL){
       return NULL;
   }
   data = (GPXData*)toBePrinted;
   char* str = "";
   str = concat("\n", data->name);
   str = concat(str, ": ");
   str = concat(str, data->value);
   return str;
}

int compareGpxData(const void *first, const void *second){

   GPXData* waypoint1;
   GPXData* waypoint2;
   if (first == NULL || second == NULL){
       return 0;
   }
   waypoint1 = (GPXData*)first;
   waypoint2 = (GPXData*)second;

   return 0;
}

void deleteGpxData(void *toBeDeleted){
   GPXData* waypoint;
   if (toBeDeleted == NULL){
       return;
   }
   waypoint = (GPXData*)toBeDeleted;
}


char* waypointToString(void *toBePrinted){
   
   Waypoint* wayPoint;

   if (toBePrinted == NULL){
       return NULL;
   }

   wayPoint = (Waypoint*)toBePrinted;
   char* str="";
   str = concat(str, "\nName: ");
   str = concat(str, wayPoint->name);
   char lat[12], lon[12];
   gcvt(wayPoint->latitude, 11, lat);
   gcvt(wayPoint->longitude, 11, lon);
   str = concat(str, "\nLat: ");
   str = concat(str, lat);
   str = concat(str, "\tLon: ");
   str = concat(str, lon);
   void* elem;
   if(wayPoint->otherData->length > 0){
       ListIterator iter = createIterator(wayPoint->otherData);
       str = concat(str, "\n\nAdditional Data");
       while((elem = nextElement(&iter)) != NULL){
           GPXData* data = (GPXData*)elem;
           char* dataS = wayPoint->otherData->printData(data);
           str = concat(str, dataS);
       }
   }
   return str;
}



int compareWaypoints(const void *first, const void *second){

   Waypoint* waypoint1;
   Waypoint* waypoint2;

   if (first == NULL || second == NULL){
       return 0;
   }

   waypoint1 = (Waypoint*)first;
   waypoint2 = (Waypoint*)second;

   return 0;
}



void deleteWaypoint(void *toBeDeleted){
   Waypoint* waypoint;
   if (toBeDeleted == NULL){
       return;
   }
   waypoint = (Waypoint*)toBeDeleted;
   if(strlen(waypoint->name)>0){
      free(waypoint->name);
    }
   clearList(waypoint->otherData);
}



char* routeToString(void *toBePrinted){

   Route* route;

   if (toBePrinted == NULL){
       return NULL;
   }
   route = (Route*)toBePrinted;
   char *str = "";
   str = concat(str, "\nname: ");
   str = concat(str, route->name);
   
   if(route->waypoints->length>0){
       str = concat(str, "\n\nWayPoints");
       ListIterator iter = createIterator(route->waypoints);
       void* elem;
       while((elem = nextElement(&iter)) != NULL){
           Waypoint* data = (Waypoint*)elem;
           char* dataS = route->waypoints->printData(data);
           str = concat(str, dataS);
       }
   }
   if(route->otherData->length>0){
       str = concat(str, "\n\nAdditional Data");
       ListIterator iter = createIterator(route->otherData);
       void* elem;
       while((elem = nextElement(&iter)) != NULL){
           GPXData* data = (GPXData*)elem;
           char* dataS = route->otherData->printData(data);
           str = concat(str, "\n");
           str = concat(str, dataS);
       }
   }
   return str;
}



int compareRoutes(const void *first, const void *second){

   Route* waypoint1;
   Route* waypoint2;

   if (first == NULL || second == NULL){
       return 0;
   }

   waypoint1 = (Route*)first;
   waypoint2 = (Route*)second;

   return 0;
}




void deleteRoute(void *toBeDeleted){
   Route* route;
   if (toBeDeleted == NULL){
       return;
   }
   route = (Route*)toBeDeleted;
    if(strlen(route->name)>0){
        free(route->name);
    }
   clearList(route->waypoints);
   clearList(route->otherData);
}




char* trackSegmentToString(void *toBePrinted){
   TrackSegment* seg;
   if (toBePrinted == NULL){
       return NULL;
   }
   seg = (TrackSegment*)toBePrinted;
   char* str = "";
   if(seg->waypoints->length>0){
       concat(str, "\n\nWaypoint");
       ListIterator iter = createIterator(seg->waypoints);
       void* elem;
       while((elem = nextElement(&iter)) != NULL){
           Waypoint* data = (Waypoint*)elem;
           char* dataS = seg->waypoints->printData(data);
           str = concat(str,dataS);
       }
   }
   return str;
}




int compareTrackSegments(const void *first, const void *second){

   TrackSegment* waypoint1;
   TrackSegment* waypoint2;

   if (first == NULL || second == NULL){
       return 0;
   }

   waypoint1 = (TrackSegment*)first;
   waypoint2 = (TrackSegment*)second;

   return 0;
}



void deleteTrackSegment(void *toBeDeleted){
   TrackSegment* seg;
   if (toBeDeleted == NULL){
       return;
   }
   seg = (TrackSegment*)toBeDeleted;
   clearList(seg->waypoints);
}


char* trackToString(void *toBePrinted){

   Track* trck;

   if (toBePrinted == NULL){
       return NULL;
   }

   trck = (Track*)toBePrinted;
   char *str = "";
   concat(str, "\nname: ");
   concat(str, trck->name);
   if(trck->segments->length>0){
       str = concat(str, "\n\nSegments");
       void* elem;
       ListIterator  iter = createIterator(trck->segments);
       while((elem = nextElement(&iter)) != NULL){
           TrackSegment* data = (TrackSegment*)elem;
           char* dataS = trck->segments->printData(data);
           str = concat(str, dataS);
       }
   }
   if(trck->otherData->length>0){
       str = concat(str, "\n\nAdditional Data");
       void* elem;
       ListIterator  iter = createIterator(trck->otherData);
       while((elem = nextElement(&iter)) != NULL){
           GPXData* data = (GPXData*)elem;
           char* dataS = trck->otherData->printData(data);
           str = concat(str, dataS);
       }
   }
   return str;
}


int compareTracks(const void *first, const void *second){

   Track* waypoint1;
   Track* waypoint2;

   if (first == NULL || second == NULL){
       return 0;
   }

   waypoint1 = (Track*)first;
   waypoint2 = (Track*)second;

   return 0;
}





void deleteTrack(void *toBeDeleted){

   Track* track;
   if (toBeDeleted == NULL){
       return;
   }
   track = (Track*)toBeDeleted;
   if(strlen(track->name)>0){
       free(track->name);
   }
   clearList(track->segments);

}


// A2 Modules

GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile){
    int valid = validateGPXfile(fileName, gpxSchemaFile);
    if(valid == 0){
        return createGPXdoc(fileName);
    }else{
        return NULL;
    }
    
}

bool writeGPXdoc(GPXdoc* doc, char* fileName){
    return writeDoc(doc, fileName);
}

bool validateGPXDoc(GPXdoc* doc, char* gpxSchemaFile){
    
    if(doc->version == 0){
        return 0;
    }
    if(doc->creator==NULL || strlen(doc->creator)<1){
        return 0;
    }
    if(doc->waypoints == NULL){
        return 0;
    }
    if(doc->namespace[0]!='h'){
        return 0;
    }
    ListIterator its = createIterator(doc->waypoints);
    void* ele;
    while((ele = nextElement(&its)) != NULL){
        if(!validateWaypoint((Waypoint*)ele)){
            return 0;
        }
    }
    
    
    if(doc->routes==NULL){
        return 0;
    }
    its = createIterator(doc->routes);
    while((ele = nextElement(&its)) != NULL){
        Route* route = (Route*) ele;
        if(route->name==NULL){
            return 0;
        }
        if(route->otherData==NULL){
            return 0;
        }
        ListIterator docIts = createIterator(route->otherData);
        void* dataEle;
        while ((dataEle = nextElement(&docIts)) != NULL) {
            if(!validateGPXData((GPXData*)dataEle)){
                return 0;
            }
        }
        docIts = createIterator(route->waypoints);
        void* rtept;
        while((rtept = nextElement(&docIts)) != NULL){
            if(!validateWaypoint((Waypoint*)rtept)){
                return 0;
            }
        }
        
    }
    
    if (doc->tracks==NULL) {
        return 0;
    }
    its = createIterator(doc->tracks);
    while((ele = nextElement(&its)) != NULL){
        Track* trak = (Track*)ele;
        if(trak->name==NULL){
            return 0;
        }
        if(trak->otherData==NULL){
            return 0;
        }
        ListIterator docIts = createIterator(trak->otherData);
        void* dataEle;
        while ((dataEle = nextElement(&docIts)) != NULL) {
            if(!validateGPXData((GPXData*)dataEle)){
                return 0;
            }
        }
        if(trak->segments==NULL){
            return NULL;
        }
        docIts = createIterator(trak->segments);
        while((dataEle = nextElement(&docIts)) != NULL){
            TrackSegment* tsg = (TrackSegment*)dataEle;
            if(tsg->waypoints == NULL){
                return 0;
            }
            ListIterator segIter = createIterator(tsg->waypoints);
            void* segwp;
            while((segwp = nextElement(&segIter)) != NULL){
                if(!validateWaypoint((Waypoint*)segwp)){
                    return 0;
                }
            }
        }


    }
    return 1;
}

float getLengthFromWaypoints(List* waypoints){
    float dist = 0;
    Node* head = waypoints->head;
    while(head->next!=NULL){
        Waypoint* source = (Waypoint*)head->data;
        Waypoint* dest = (Waypoint*)head->next->data;
        float d = calcDistance(source->latitude, source->longitude, dest->latitude, dest->longitude);
        dist += d;
        head=head->next;
        //printf("\n%f, %f -> %f, %f => %f\n",source->latitude, source->longitude, dest->latitude, dest->longitude, d);
    }
    return dist*1000;
}

float getRouteLen(const Route *rt){
    //calcDistance
    double dist=0;
    return getLengthFromWaypoints(rt->waypoints);
}


float getTrackLen(const Track *tr){
    double dist=0;
    Node* head = tr->segments->head;
    int first = 1;
    double lastLat = 0;
    double lastLon = 0;
    while(head!=NULL){
        TrackSegment* seg = (TrackSegment*)head->data;
        if(first!=1){
            Node* first = seg->waypoints->head;
            double firstLat = ((Waypoint*)first)->latitude;
            double firstLon = ((Waypoint*)first)->longitude;
            float sl = calcDistance(lastLat,lastLat, firstLat, firstLon);
            dist+=sl;
        }else{
            first = 0;
        }
        float f = getLengthFromWaypoints(seg->waypoints);
        dist += f;
        Node* last = seg->waypoints->tail;
        lastLat = ((Waypoint*)last)->latitude;
        lastLat = ((Waypoint*)last)->longitude;
        head=head->next;
        //printf("\n%f, %f -> %f, %f => %f\n",source->latitude, source->longitude, dest->latitude, dest->longitude, d);
    }
    return dist*1000;
}


float round10(float len){
    int l =ceil(len);
    int r = l%10;
    if(r<=5){
        l-=r;
    }else{
        l+=(10-r);
    }
    return l;
}


int numRoutesWithLength(const GPXdoc* doc, float len, float delta){
    return 0;
}


int numTracksWithLength(const GPXdoc* doc, float len, float delta){
    return 0;
}

bool isLoopRoute(const Route* route, float delta){
    return 0;
}

bool isLoopTrack(const Track *tr, float delta){
    return 0;
}


List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta){
    return NULL;
}

List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta){
    return NULL;
}

bool validateWaypoint(Waypoint* nd){
    void* dataEle;
    if(nd->name==NULL){
        return 0;
    }
    if(nd->latitude == 0 || nd->longitude == 0){
        return 0;
    }
    if(nd->otherData==NULL){
        return 0;
    }
    ListIterator docIts = createIterator(nd->otherData);
    while ((dataEle = nextElement(&docIts)) != NULL) {
        if(!validateGPXData((GPXData*)dataEle)){
            return 0;
        }
    }
    return 1;
}

bool validateGPXData(GPXData* dt){
    if(strlen(dt->name)<1 || strlen(dt->value) < 1){
        return 0;
    }
    return 1;
}

void builtWayPoint(xmlNodePtr parent_node, Waypoint* nd){
    xmlNodePtr node1 = NULL, node2 = NULL;
    char latS[12], lonS[12];
    gcvt(nd->latitude, 11, latS);
    gcvt(nd->longitude, 11, lonS);
    xmlNewProp(parent_node, BAD_CAST "lat", BAD_CAST latS);
    xmlNewProp(parent_node, BAD_CAST "lon", BAD_CAST lonS);
    ListIterator docIts = createIterator(nd->otherData);
    void* dataEle;
    if(nd->name!=NULL && strlen(nd->name)>0){
        node1 = xmlNewChild(parent_node, NULL, BAD_CAST "name", NULL);
        node2 = xmlNewText(BAD_CAST nd->name);
        xmlAddChild(node1, node2);
    }
    while ((dataEle = nextElement(&docIts)) != NULL) {
        GPXData* doc = (GPXData*) dataEle;
        node1 = xmlNewChild(parent_node, NULL, BAD_CAST doc->name, NULL);
        node2 = xmlNewText(BAD_CAST doc->value);
        xmlAddChild(node1, node2);
    }
}
int writeDoc(GPXdoc* gpxdoc, char *file)
{
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, node2 = NULL;/* node pointers */

    LIBXML_TEST_VERSION;

    /*
     * Creates a new document, a node and set it as a root node
     */
    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "gpx");
    if(gpxdoc->creator!=NULL && strlen(gpxdoc->creator)>0){
        xmlNewProp(root_node, BAD_CAST "creator", BAD_CAST gpxdoc->creator);
    }
    if(gpxdoc->version!=0){
        char version[4];
        gcvt(gpxdoc->version, 4, version);
        xmlNewProp(root_node, BAD_CAST "version", BAD_CAST version);
    }
    if(gpxdoc->namespace[0] == 'h'){
        xmlNewProp(root_node, BAD_CAST "xmlns", BAD_CAST gpxdoc->namespace);
    }
       
    xmlDocSetRootElement(doc, root_node);

    /*
     * Creates a DTD declaration. Isn't mandatory.
     */
    //xmlCreateIntSubset(doc, BAD_CAST "root", NULL, BAD_CAST "tree2.dtd");

    /*
     * xmlNewChild() creates a new node, which is "attached" as child node
     * of root_node node.
     */
    ListIterator its = createIterator(gpxdoc->waypoints);
    void* ele;
    while((ele = nextElement(&its)) != NULL){
        node =
            xmlNewChild(root_node, NULL, BAD_CAST "wpt",
                        BAD_CAST NULL);
        builtWayPoint(node, (Waypoint*)ele);

    }
    
    its = createIterator(gpxdoc->routes);
    while((ele = nextElement(&its)) != NULL){
        Route* route = (Route*) ele;
        node =
            xmlNewChild(root_node, NULL, BAD_CAST "rte",
                        BAD_CAST NULL);
        if(route->name!=NULL && strlen(route->name)>0){
            node1 = xmlNewChild(node, NULL, BAD_CAST "name", NULL);
            node2 = xmlNewText(BAD_CAST route->name);
            xmlAddChild(node1, node2);
        }
        ListIterator docIts = createIterator(route->otherData);
        void* dataEle;
        while ((dataEle = nextElement(&docIts)) != NULL) {
            GPXData* doc = (GPXData*) dataEle;
            node1 = xmlNewChild(node, NULL, BAD_CAST doc->name, NULL);
            node2 = xmlNewText(BAD_CAST doc->value);
            xmlAddChild(node1, node2);
        }
        docIts = createIterator(route->waypoints);
        void* rtept;
        while((rtept = nextElement(&docIts)) != NULL){
            node1 =
                xmlNewChild(node, NULL, BAD_CAST "rtept",
                            BAD_CAST NULL);
            builtWayPoint(node1, (Waypoint*)rtept);

        }
        
    }
    
    its = createIterator(gpxdoc->tracks);
    while((ele = nextElement(&its)) != NULL){
        Track* trak = (Track*) ele;
        node =
            xmlNewChild(root_node, NULL, BAD_CAST "trk",
                        BAD_CAST NULL);
        if(trak->name!=NULL && strlen(trak->name)>0){
            node1 = xmlNewChild(node, NULL, BAD_CAST "name", NULL);
            node2 = xmlNewText(BAD_CAST trak->name);
            xmlAddChild(node1, node2);
        }
        ListIterator docIts = createIterator(trak->otherData);
        void* dataEle;
        while ((dataEle = nextElement(&docIts)) != NULL) {
            GPXData* doc = (GPXData*) dataEle;
            node1 = xmlNewChild(node, NULL, BAD_CAST doc->name, NULL);
            node2 = xmlNewText(BAD_CAST doc->value);
            xmlAddChild(node1, node2);
        }
        docIts = createIterator(trak->segments);
        while((dataEle = nextElement(&docIts)) != NULL){
            TrackSegment* seg = (TrackSegment*) dataEle;
            node1 =
                xmlNewChild(node, NULL, BAD_CAST "trkseg",
                            BAD_CAST NULL);
            ListIterator segWayIter = createIterator(seg->waypoints);
            void* segwap;
            while((segwap = nextElement(&segWayIter)) != NULL){
                node2 =
                    xmlNewChild(node1, NULL, BAD_CAST "trkpt",
                                BAD_CAST NULL);
                builtWayPoint(node2,(Waypoint*)segwap);
            }
        }


    }
    /*
     * Dumping document to stdio or file
     */
    xmlSaveFormatFileEnc(file, doc, "UTF-8", 1);

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();
    return(0);
}


int validateGPXfile(char *XMLFileName, char *XSDFileName)
{
    xmlDocPtr doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;
    xmlLineNumbersDefault(1);
    ctxt = xmlSchemaNewParserCtxt(XSDFileName);
    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);
    doc = xmlReadFile(XMLFileName, NULL, 0);
    int ret = -1;
    if (doc == NULL)
    {
        fprintf(stderr, "Could not parse %s\n", XMLFileName);
    }
    else
    {
        xmlSchemaValidCtxtPtr ctxt;
        ctxt = xmlSchemaNewValidCtxt(schema);
        xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
        ret = xmlSchemaValidateDoc(ctxt, doc);
        xmlSchemaFreeValidCtxt(ctxt);
        xmlFreeDoc(doc);
    }

// free the resource
    if(schema != NULL)
        xmlSchemaFree(schema);

    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();

    return(ret);
}



// Assigment2 Module 3


char* trackToJSON(const Track *tr){
    
    char* json;
    json = "";
    concat(json, "{");
    if(strlen(tr->name)>0){
        json = concat(json, "{ \"name\": \"");
        json = concat(json, tr->name);
        if(tr->otherData->length>0 || tr->segments->length>0){
            json = concat(json, "\", ");
        }
    }
    if(tr->otherData->length > 0){
        ListIterator dit = createIterator(tr->otherData);
        void* dn;
        int i = 0;
        while((dn = nextElement(&dit))!=NULL){
            json = concat(json, gpxDataToJson((GPXData*)dn));
            if(i<tr->otherData->length-1){
                json = concat(json, ", ");
            }
            i++;
        }
        if(tr->segments->length > 0){
            json = concat(json, ", ");
        }
    }
    if(tr->segments->length>0){
        int segCount = 0;
        ListIterator segIter = createIterator(tr->segments);
        void* el;
        while((el = nextElement(&segIter))!=NULL){
            TrackSegment* seg = (TrackSegment*) el;
            json = concat(json, "\" segments\": [");
            if(seg->waypoints->length>0){
                json = concat(json, " {\"waypoints\": [ ");
                ListIterator wp = createIterator(seg->waypoints);
                void* el2;
                int i=0;
                while((el2 = nextElement(&wp))!=NULL){
                    Waypoint* wp = (Waypoint*) el2;
                    json = concat(json, wayPointToJson(wp));
                    if(i<seg->waypoints->length-1){
                        json = concat(json, ", ");
                    }
                    i++;
                }
                json = concat(json, "]");
                json = concat(json, " }");
            }
            if(segCount<tr->segments->length-1){
                json = concat(json, ", ");
            }
            segCount++;
        }
        json = concat(json, "]");
    }
    json = concat(json, "}");
    return json;
    
}

char* gpxDataToJson(GPXData* data){
    char* json = "";
    json = concat(json,"\"");
    json = concat(json, data->name);
    json = concat(json, "\": \"");
    json = concat(json, data->value);
    json = concat(json, "\"");
    return json;
}
char* wayPointToJson(Waypoint* wp){
    char* json = "{ ";
    
    if(strlen(wp->name)>0){
        json = concat(json, "\"name\": \"");
        json = concat(json, wp->name);
        json = concat(json, "\" ");
        json = concat(json, ", ");
    }
        
    char latS[12], lonS[12];
    gcvt(wp->latitude, 11, latS);
    gcvt(wp->longitude, 11, lonS);
    
    json = concat(json, "\"latitude\": ");
    json = concat(json, latS);
    json = concat(json, ", \"longitude\": ");
    json = concat(json, lonS);
    
    if(wp->otherData->length > 0){
        json = concat(json, ", ");
        ListIterator dit = createIterator(wp->otherData);
        void* dn;
        int i = 0;
        while((dn = nextElement(&dit))!=NULL){
            json = concat(json, gpxDataToJson((GPXData*)dn));
            if(i< wp->otherData->length-1){
                json = concat(json, ", ");
            }
            i++;
        }
    }
    json = concat(json, "}");
    return json;
}

char* routeToJSON(const Route *rt){
    
    char* json = "";
    if(strlen(rt->name)>0){
        json = concat(json, "{ \"name\": \"");
        json = concat(json, rt->name);
        json = concat(json, "\"");
        if(rt->otherData->length > 0 || rt->waypoints->length > 0){
            json = concat(json, ", ");
        }
    }
    if(rt->otherData->length > 0){
        ListIterator dit = createIterator(rt->otherData);
        void* dn;
        int i = 0;
        while((dn = nextElement(&dit))!=NULL){
            json = concat(json, gpxDataToJson((GPXData*)dn));
            if(i<rt->otherData->length-1){
                json = concat(json, ", ");
            }
        }
        if(rt->waypoints->length>0){
            json = concat(json, ", ");
        }
    }
    
    if(rt->waypoints->length > 0){
        json = concat(json, "\"rtept\": [");
        ListIterator its = createIterator(rt->waypoints);
        void* ele;
        int i=0;
        while((ele = nextElement(&its))!=NULL){
            Waypoint* wayPoint = (Waypoint*) ele;
            json = concat(json, wayPointToJson(wayPoint));
            if(i < rt->waypoints->length-1){
                json = concat(json, ", ");
            }
            i++;
        }
        json = concat(json, " ]");
    }
    json = concat(json, " }");
    return json;
}

char* routeListToJSON(const List *list){
    char* json;
    json = "[ ";
    void* item;
    int i=0;
    ListIterator its = createIterator(list);
    while((item = nextElement(&its))!=NULL){
        json = concat(json, routeToJSON((Route*)item));
        if(i<list->length-1){
            json = concat(json, ",");
        }
        i++;
    }
    json = concat(json," ]");
    return json;
}

char* trackListToJSON(const List *list){
    
    char* json;
    json = "[ ";
    ListIterator its = createIterator(list);
    void* item;
    int i=0;
    while((item = nextElement(&its))!=NULL){
        json = concat(json, trackToJSON((Track*)item));
        if(i<list->length-1){
            json = concat(json, ",");
        }
        i++;
    }
    json = concat(json," ]");
    return json;
}


char* GPXtoJSON(const GPXdoc* gpx){
    
    char* json = "{";
    json = concat(json, "\"creator\": \"");
    json = concat(json, gpx->creator);
    json = concat(json, "\"");
    if(gpx->namespace[0]=='h'){
        json = concat(json, ", \"namespace\": \"");
        json = concat(json, gpx->namespace);
        json = concat(json, "\"");
    }
    if(gpx->version>0){
        json = concat(json, ", \"version\": ");
        char lat[5];
        gcvt(gpx->version, 4, lat);
        json = concat(json, lat);
    }
    
    json = concat(json, ", ");
    
    if(gpx->waypoints->length>0){
        json = concat(json, "\"wpt\": [");
        ListIterator its = createIterator(gpx->waypoints);
        void* item;
        int i = 0;
        while((item = nextElement(&its))!=NULL){
            json = concat(json, wayPointToJson((Waypoint*)item));
            if(i<gpx->waypoints->length-1){
                json = concat(json, ", ");
            }
            i++;
        }
        json = concat(json, " ]");
    }else{
        json = concat(json, "\"wpt\": []");
    }
            
    json = concat(json, ", ");
    
    if(gpx->routes->length>0){
        json = concat(json, "\"rte\": ");
        json = concat(json, routeListToJSON(gpx->routes));
    }
    else{
        json = concat(json, "\"rte\": []");
    }
    
    json = concat(json, ", ");
    
    if(gpx->tracks->length>0){
        json = concat(json, "\"trk\": ");
        json = concat(json, trackListToJSON(gpx->tracks));
    }else{
        json = concat(json, "\"trk\": []");
    }
    
    json = concat(json, "}");
    return json;
}





float calcDistance(double latHome, double lonHome, double latDest, double lonDest) {

    double pi = 3.141592653589793;
    int R = 6371; //Radius of the Earth
    latHome = (pi/180)*(latHome);
    latDest = (pi/180)*(latDest);
    double differenceLon = (pi/180)*(lonDest - lonHome);
    double differenceLat = (pi/180)*(latDest - latHome);
//    double a = sin(differenceLat/2) * sin(differenceLat/2) +
//    cos(latHome) * cos(latDest) *
//    sin(differenceLon/2) * sin(differenceLon/2);
    double latHomeTmp = (pi/180)*(latHome);
    double latDestTmp = (pi/180)*(latDest);
    double a = sin(differenceLat/2.) * sin(differenceLat/2.) +
               cos(latHomeTmp) * cos(latDestTmp) *
               sin(differenceLon/2.) * sin(differenceLon/2.);
    
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double distance = R * c;
    return distance;
}
