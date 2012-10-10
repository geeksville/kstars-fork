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
                    guidesView.guidesClicked(index);
                    guidesView.currentIndex = index//guidesView.indexAt(mouseX, mouseY)
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

    //    Component {
    //        id:slidesdelegate

    //        Item {
    //            id: slides
    //            height: window.height - guidesView.y
    //            width: sdelegate.ListView.view.width

    //            Rectangle {
    //                id:slideText
    //                color: "#ffffff"
    //                x:30
    //                height:parent.height -100
    //                width: parent.width

    //            }

    //        }
    //    }

    //    Component{
    //        id:slidesdelegate

    //        Item {
    //            id: sdelegate
    //            height: scolumn.childrenRect.height
    //            width: sdelegate.ListView.view.width

    //            Column{
    //                id:scolumn
    //                //width: parent.width-40;
    //                anchors.left: parent.left
    //                anchors.right: parent.right

    //                Text {
    //                    id: description
    //                    text: model.title
    //                }

    //                //                Rectangle {
    //                //                    id:viewimages
    //                //                    height: 20
    //                //                    width: 70
    //                //                    color: "#000000"

    //                //                    Text {
    //                //                        id: backtext
    //                //                        text: "View Images"
    //                //                        color: "#ffffff"
    //                //                    }

    //                //                    MouseArea {
    //                //                        id:view
    //                //                        anchors.fill: parent
    //                //                        onClicked: {
    //                //                            slidesView.viewImagesClicked(index);
    //                //                        }
    //                //                    }

    //                //                }
    ////                    ListView {
    ////                    id:inner
    ////                    model: model1
    ////                    delegate: delegate2
    ////                    contentHeight: contentItem.childrenRect.height
    ////                    height: childrenRect.height
    ////                    anchors.left: parent.left
    ////                    anchors.right: parent.right
    ////                    clip: true
    ////                }
    //            }

    //            Rectangle {
    //                width: parent.width;
    //                height: 2 * sizefactor;
    //                color: "#000000"
    //                anchors.bottom: slidesdelegate.bottom
    //            }
    //        }
    //    }


    //    ListModel {
    //        id:model1

    //        ListElement {
    //            name: "/home/rmr/images.jpeg"
    //        }
    //        ListElement {
    //            name: "/home/rmr/images.jpeg"
    //        }
    //        ListElement {
    //            name: "/home/rmr/images.jpeg"
    //        }
    //    }

    //    Component {
    //        id: delegate2

    //        Item {
    //            width: 100 * sizefactor;
    //            height: col2.childrenRect.height

    //            MouseArea {
    //                id:mclick
    //                anchors.fill: parent
    //                onClicked: {
    //                    console.log("hai....")
    //                    //guidesView.addNewGuidesClicked()
    //                           // slidesView.viewImagesClicked(index);
    //                    guidesView.viewImagesClicked(parent.index,index);
    //                }
    //            }

    //            Column {
    //                id: col2
    //                anchors.left: parent.left
    //                anchors.right: parent.right
    ////                Text {
    ////                    id: name1
    ////                    text: name
    ////                }
    //                Image {
    //                    id: iname
    //                    width: 50 * sizefactor;
    //                    height: 50 * sizefactor;
    //                    source: name


    //                    }
    //                }

    //            }
    //        }
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
                text: slidetext//"Hello There ! is it working ?"
            }

        }

        Rectangle {
            id: nextSlide
            //anchors.left: parent.left
            //anchors.centerIn: parent.Center
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
          //  anchors.right: parent.right
          //  anchors.centerIn: parent.Center
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
            //            model: ListModel {
            //                ListElement {
            //                    name: "Bill Smith"
            //                    number: "555 3264"
            //                }
            //                ListElement {
            //                    name: "John Brown"
            //                    number: "555 8426"
            //                }
            //                ListElement {
            //                    name: "Sam Wise"
            //                    number: "555 0473"
            //                }
            //            }
            model : imageModel
            //             delegate: Text {
            //                text: name + ": " + number + (index == list.count-1 ? "" : ",")
            //            }
            delegate: Image {
                id: slideImage
                width: 50
                height: 50
                source: model.image
            }
            highlight: Rectangle {
                width: imageList.currentItem.width
                color: "lightsteelblue"
                radius: 5
            }
            focus: true
            MouseArea {
                anchors.fill:  parent
                onClicked: {
                    imageList.currentIndex = imageList.indexAt(mouseX, mouseY)
                }
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
        signal viewImagesClicked(int pindex,int index);
        signal backButtonClicked();
        signal nextSlideClicked();
        signal previousSlideClicked();

    }

    //    ListView {

    //        id: slidesView
    //        objectName: "slideslist"
    //        y:50 * sizefactor;
    //        height: parent.height-50
    //        width: parent.width
    //        model: slidesmodel
    //        delegate: slidesdelegate
    //        clip: true
    //        opacity: window.slidesOpacity


    //    }



    //    Rectangle {
    //        id:backbutton
    //        height: 20 * sizefactor;
    //        width: 40 * sizefactor;
    //        color: "#000000"
    //        x:10 * sizefactor;
    //        y:10 * sizefactor;
    //        opacity: window.slidesOpacity

    //        Text {
    //            id: backtext
    //            text: "Back"
    //            color: "#ffffff"
    //        }

    //        MouseArea {
    //            id:backbuttonclick
    //            anchors.fill: parent
    //            onClicked: {
    //                guidesView.backButtonClicked();
    //                window.guidesOpacity=1;
    //                window.slidesOpacity=0;
    //            }
    //        }
    //    }

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
