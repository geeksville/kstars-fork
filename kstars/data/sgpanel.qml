import QtQuick 1.0

Rectangle {
    id: window
    objectName: "base"
    width: 300
    height: 600
    color: "#000000"
    opacity: 0.6

    property int slidesOpacity: 0
    property int guidesOpacity: 1

    Component {
        id: feeddelegate
        
        Item {
            id: delegate
            height: column.height + 40
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
                    width: 50
                    height: 50
                    source: thumbimage
                }

                Column {
                    id: column
                    x: 20; y: 20
                    width: parent.width - 40

                    Text {
                        id: title
                        text: model.title; width: parent.width; wrapMode: Text.WordWrap
                        font { bold: true; family: "Helvetica"; pointSize: 10 }
                    }



                }
            }

            Rectangle {
                width: parent.width; height: 2; color: "#000000"
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
                width: parent.width; height: 2; color: "#000000"
                anchors.bottom: slidesdelegate.bottom
            }
        }
    }


    ListModel {
        id:model1

        ListElement {
            name: trial_img1
        }
        ListElement {
            name: trial_img2
        }
        ListElement {
            name: trial_img3
        }
    }

    Component {
        id: delegate2

        Item {
            width: 100
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
                    width: 50
                    height: 50
                    source: name


                    }
                }

            }
        }


    ListView {

        id: guidesView
        objectName: "guidesList"
        y:40
        width: parent.width
        height: parent.height-40
        model: feedModel
        delegate: feeddelegate
        clip: true
        opacity: window.guidesOpacity;
        
        signal guidesClicked(int index);
        signal addNewGuidesClicked();
        signal viewImagesClicked(int pindex,int index);

    }

    ListView {

        id: slidesView
        objectName: "slideslist"
        y:50
        height: parent.height-50
        width: parent.width
        model: slidesmodel
        delegate: slidesdelegate
        clip: true
        opacity: window.slidesOpacity
        signal backButtonClicked();

    }



    Rectangle {
        id:backbutton
        height: 20
        width: 40
        color: "#000000"
        x:10
        y:10
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
                slidesView.backButtonClicked();
                window.guidesOpacity=1;
                window.slidesOpacity=0;
            }
        }
    }

    Rectangle {
        id:addnewguides
        objectName: "newbutton"
        height: 20
        width: 40
        color: "#000000"
        x:10
        y:10
        opacity: window.guidesOpacity



        Text {
            id: addnew
            text: "New"
            color: "#ffffff"
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



