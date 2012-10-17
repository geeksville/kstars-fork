import QtQuick 1.0

Rectangle {
    id: window
    objectName: "base"
    width: w   //300 * sizefactor;
    height: h  //600 * sizefactor;
    color: "#00FFFFFF"
    opacity: 0.6

    property double sizefactor: (window.width/300);
    // property int slidesOpacity: 0
    property int guidesOpacity: 0
    property int sideopacity: 1

    Image {
        id: bg
        source:background
        height :window.height
        width : window.width
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



    Component {
        id: feeddelegate
        
        Item {
            id: delegate
            height: column.height + dimage.height
            width: delegate.ListView.view.width

            //            MouseArea {
            //                anchors.fill: parent
            //                onClicked:{
            //                    //guidesView.guidesClicked(index);
            //                    //window.guidesOpacity=0;
            //                    //window.slidesOpacity=1;
            //                }
            //            }

            MouseArea {
                anchors.fill:  parent
                onClicked: {
                    guidesView.guidesClicked(index)
                    guidesView.currentIndex = index
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
                        font { bold: true; family: "Helvetica";  }
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

    Item {
        id: slideTextItem

        Rectangle {
            id: slideTextRect
            y:50
            x:40
            height: guidesView.y-150
            width: window.width-90
            color: "#ffffff"
            opacity: guidesOpacity

            Text {
                id: slideText
                text: slidetext
            }

        }

        Rectangle {
            id: nextSlide
            x:slideTextRect.width+50
            y:slideTextRect.height-(nextSlide.height/2)-40
            width: nextSlideText.width + 10
            height: nextSlideText.height + 10
            radius: 3
            border.width: 1
            smooth: true
            border.color: "#000000"
            opacity: guidesOpacity
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#5e5e5e"
                }
                GradientStop {
                    position: 1
                    color: "#444444"
                }
            }
            Text {
                x: 5
                y: 5
                id: nextSlideText
                text: ">"
                color: "#ffffff"
            }
            MouseArea {
                anchors.fill:  parent
                onClicked: {
                    guidesView.nextSlideClicked();
                }
            }
        }

        Rectangle {
            id: previousSlide
            x:slideTextRect.x-30
            y:slideTextRect.height-(nextSlide.height/2)-40
            width: previousSlideText.width + 10
            height: previousSlideText.height + 10
            radius: 3
            border.width: 1
            smooth: true
            border.color: "#000000"
            opacity: guidesOpacity
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#5e5e5e"
                }
                GradientStop {
                    position: 1
                    color: "#444444"
                }
            }
            Text {
                x: 5
                y: 5
                id: previousSlideText
                text: "<"
                color: "#ffffff"
            }
            MouseArea {
                anchors.fill:  parent
                onClicked: {
                    guidesView.previousSlideClicked();
                }
            }
        }
    }


    Item {
        y:slideTextRect.height+70
        width: window.width
        height: 100
        opacity:guidesOpacity

        Rectangle {
            id: buttonLeft
            anchors.left: parent.left
            width: buttonLeftText.width + 10
            height: buttonLeftText.height + 10
            radius: 3
            border.width: 1
            smooth: true
            border.color: "#000000"
            opacity: guidesOpacity
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#5e5e5e"
                }
                GradientStop {
                    position: 1
                    color: "#444444"
                }
            }
            Text {
                x: 5
                y: 5
                id: buttonLeftText
                text: "<"
                color: "#ffffff"
            }
            MouseArea {
                anchors.fill:  parent
                onClicked: {
                    imageList.currentIndex = Math.max(0, imageList.currentIndex-1)

                }
            }
        }

        Rectangle {
            id: buttonRight
            anchors.right: parent.right
            width: buttonRightText.width + 10
            height: buttonRightText.height + 10
            radius: 3
            border.width: 1
            smooth: true
            border.color: "#000000"
            opacity: guidesOpacity
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#5e5e5e"
                }
                GradientStop {
                    position: 1
                    color: "#444444"
                }
            }
            Text {
                x: 5
                y: 5
                id: buttonRightText
                text: ">"
                color: "#ffffff"
            }
            MouseArea {
                anchors.fill:  parent
                onClicked: {
                    imageList.currentIndex = Math.min(imageList.count-1, imageList.currentIndex+1)
                }
            }
        }

        ListView {
            id: imageList
            clip: true
            spacing: 5
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right:  buttonRight.left
            anchors.left:  buttonLeft.right
            opacity: guidesOpacity
            orientation: ListView.Horizontal
            model : imageModel
            delegate:imageDelegate
            highlight: Rectangle {
                width: imageList.currentItem.width
                color: "lightsteelblue"
                radius: 5
            }
            focus: true
            //            MouseArea {
            //                anchors.fill:  parent
            //                onClicked: {
            //                    imageList.currentIndex = imageList.indexAt(mouseX, mouseY)
            //                }
            //            }
        }
    }

    Component {
        id: imageDelegate

        Item {
            id: imageItem
            width: 50
            height: 50

            MouseArea {
                id: imageArea
                anchors.fill: parent
                onClicked: {

                    imageList.currentIndex = index
                   guidesView.viewImagesClicked(index)
                }
            }

            Image {
                id: slideImage
                width: 50
                height: 50
                source: model.image
            }


        }
    }



    ListView {

        id: guidesView
        objectName: "guidesList"
        y:350 * sizefactor;
        width: parent.width
        height: parent.height-guidesView.y-20
        model: feedModel
        delegate: feeddelegate
        clip: true
        opacity: window.guidesOpacity;
        highlight: Rectangle {
            width: guidesView.currentItem.width
            color: "lightsteelblue"
            radius: 5
        }
        focus: true

        //        MouseArea {
        //            anchors.fill:  parent
        //            onClicked: {
        //                guidesView.currentIndex = guidesView.indexAt(mouseX, mouseY)
        //            }
        //        }
        
        signal guidesClicked(int index);
        signal addNewGuidesClicked();
        signal viewImagesClicked(int index);
        signal backButtonClicked();
        signal nextSlideClicked();
        signal previousSlideClicked();

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
            font { bold: true; family: "Helvetica"; }

            MouseArea {
                id:addnewbutton
                anchors.fill: parent
                onClicked: {
                    guidesView.addNewGuidesClicked();
                }
            }
        }
    }
}
