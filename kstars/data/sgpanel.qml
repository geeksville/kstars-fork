import QtQuick 1.0

Rectangle {
    id: window
    objectName: "base"
    width: w   //300 * sizefactor;
    height: h  //600 * sizefactor;
    color: "#00FFFFFF"
    opacity: 0.6

    Image {
        id: bg
        source:background
	x:-5
        height :parent.height
        width : parent.width
        opacity : guidesOpacity
    }
    
    Rectangle {
    id: minimize
    x:window.width - 50;
    y: 25
    width: 20   //300 * sizefactor;
    height: 5  //600 * sizefactor;
    color: "#ffffff"
    opacity: guidesOpacity
    
    MouseArea
        {
         id:minbutton
         anchors.fill: parent
         onClicked: {
             sideopacity =1 ;
             guidesOpacity = 0;
         }
    
    }
    }
      

    Image {
        id: side
        source:sgsideselect
        x:0
        height : 200 * sizefactor;
        width : 40 * sizefactor;
        y:(window.height/2)-(side.height/2)
	
        opacity: sideopacity

        MouseArea
        {
         id:sideselect
         anchors.fill: parent
         onClicked: {
             sideopacity =0 ;
             guidesOpacity = 1;
         }
        }
    }

    property double sizefactor: (window.width/300);
    property int slidesOpacity: 0
    property int guidesOpacity: 0
    property int sideopacity: 1

    Component {
        id: feeddelegate
        
        Item {
            id: delegate
            height: column.height + dimage.height
            width: delegate.ListView.view.width

            MouseArea {
                anchors.fill: parent
                onClicked:{
                    guidesView.guidesClicked(index);
                    window.guidesOpacity=0;
                    window.slidesOpacity=1;
                }
            }

            Row {
                id:drow
                width: parent.width
                height: parent.height

                Image {
                    id: dimage
                    width: 50 * sizefactor;
                    height: 50 * sizefactor;
                    source: model.thumbimage
                }

                Column {
                    id: column
                    x: 20 * sizefactor;
                    y: 20 * sizefactor;
                    width: parent.width - 40

                    Text {
                        id: title
                        text: model.title; width: parent.width; wrapMode: Text.WordWrap
                        font { bold: true; family: "Helvetica"; pointSize: 10*sizefactor; }
                         color:"#ffffff"
                    }



                }
            }

            Rectangle {
                width: parent.width;
                height: 2 * sizefactor;
                color: "#ffffff"
                opacity: 0.4
                anchors.bottom: feeddelegate.bottom
            }

        }

    }

    Component{
        id:slidesdelegate

        Item {
            id: sdelegate
            height: scolumn.childrenRect.height
            width: sdelegate.ListView.view.width

            Column{
                id:scolumn
                //width: parent.width-40;
                anchors.left: parent.left
                anchors.right: parent.right
                
                Text {
                    id: description
                    text: model.title
                }

                //                Rectangle {
                //                    id:viewimages
                //                    height: 20
                //                    width: 70
                //                    color: "#000000"

                //                    Text {
                //                        id: backtext
                //                        text: "View Images"
                //                        color: "#ffffff"
                //                    }

                //                    MouseArea {
                //                        id:view
                //                        anchors.fill: parent
                //                        onClicked: {
                //                            slidesView.viewImagesClicked(index);
                //                        }
                //                    }

                //                }
                ListView {
                    id:inner
                    model: model1
                    delegate: delegate2
                    contentHeight: contentItem.childrenRect.height
                    height: childrenRect.height
                    anchors.left: parent.left
                    anchors.right: parent.right
                    clip: true
                }
            }

            Rectangle {
                width: parent.width;
                height: 2 * sizefactor;
                color: "#000000"
                anchors.bottom: slidesdelegate.bottom
            }
        }
    }


    ListModel {
        id:model1

        ListElement {
            name: "/home/rmr/images.jpeg"
        }
        ListElement {
            name: "/home/rmr/images.jpeg"
        }
        ListElement {
            name: "/home/rmr/images.jpeg"
        }
    }

    Component {
        id: delegate2

        Item {
            width: 100 * sizefactor;
            height: col2.childrenRect.height

            MouseArea {
                id:mclick
                anchors.fill: parent
                onClicked: {
                    console.log("hai....")
                    //guidesView.addNewGuidesClicked()
                           // slidesView.viewImagesClicked(index);
                    guidesView.viewImagesClicked(parent.index,index);
                }
            }

            Column {
                id: col2
                anchors.left: parent.left
                anchors.right: parent.right
//                Text {
//                    id: name1
//                    text: name
//                }
                Image {
                    id: iname
                    width: 50 * sizefactor;
                    height: 50 * sizefactor;
                    source: name


                    }
                }

            }
        }


    ListView {

        id: guidesView
        objectName: "guidesList"
        y:40 * sizefactor;
        width: parent.width
        height: parent.height-40
        model: feedModel
        delegate: feeddelegate
        clip: true
        opacity: window.guidesOpacity;
        
        signal guidesClicked(int index);
        signal addNewGuidesClicked();
        signal viewImagesClicked(int pindex,int index);
        signal backButtonClicked();

    }

    ListView {

        id: slidesView
        objectName: "slideslist"
        y:50 * sizefactor;
        height: parent.height-50
        width: parent.width
        model: slidesmodel
        delegate: slidesdelegate
        clip: true
        opacity: window.slidesOpacity


    }



    Rectangle {
        id:backbutton
        height: 20 * sizefactor;
        width: 40 * sizefactor;
        color: "#000000"
        x:10 * sizefactor;
        y:10 * sizefactor;
        opacity: window.slidesOpacity

        Text {
            id: backtext
            text: "Back"
            color: "#ffffff"
        }

        MouseArea {
            id:backbuttonclick
            anchors.fill: parent
            onClicked: {
                guidesView.backButtonClicked();
                window.guidesOpacity=1;
                window.slidesOpacity=0;
            }
        }
    }

    Rectangle {
        id:addnewguides
        objectName: "newbutton"
        height: 20 * sizefactor;
        width: 40 * sizefactor;
        color: "#00FFFFFF"
        x:10 * sizefactor;
        y:10* sizefactor;
        opacity: window.guidesOpacity



        Text {
            id: addnew
            text: "NEW"
	    y:10 * sizefactor
            color: "#ffffff"
	    font { bold: true; family: "Helvetica"; pointSize: 10*sizefactor; }
        }

        MouseArea {
            id:addnewbutton
            anchors.fill: parent
            onClicked: {
                guidesView.addNewGuidesClicked();
            }
        }
    }


}



