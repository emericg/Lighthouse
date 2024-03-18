import QtQuick3D

View3D { // pokeball 3d
    id: pokeballView

    width: parent.width
    height: width

    property real xx: 0.0
    property real yy: 0.0
    property real zz: 0.0

    property real joyx: 0.0
    property real joyy: 0.0

    //! [environment]
    environment: SceneEnvironment {
        //clearColor: Theme.colorBackground
        //backgroundMode: Theme.colorBackground

        antialiasingMode: SceneEnvironment.MSAA
        antialiasingQuality: SceneEnvironment.High
        temporalAAEnabled: false
        temporalAAStrength: 1
    }
    //! [environment]


    //! [camera]
    PerspectiveCamera {
        position: Qt.vector3d(0, 200, 300)
        eulerRotation: Qt.vector3d(-30, 0, 0)
    }

    //! [camera]


    //! [light]
    DirectionalLight {
        eulerRotation.x: -30
        eulerRotation.y: -70
    }
    DirectionalLight {
        eulerRotation.x: 30
        eulerRotation.y: 70
    }
    //! [light]


    Pokeball3dModel {
        position: Qt.vector3d(0, 0, 0)
        scale: Qt.vector3d(1, 1, 1)

        eulerRotation: Qt.vector3d(pokeballView.xx, pokeballView.yy, pokeballView.zz)
    }


/*
    //! [objects]
    Model {
        position: Qt.vector3d(0, 0, 0)
        source: "#Cylinder"
        scale: Qt.vector3d(2, 0.2, 1)
        materials: [ DefaultMaterial {
                diffuseColor: "red"
            }
        ]
    }

    Model {
        position: Qt.vector3d(0, 150, 0)
        source: "#Sphere"

        materials: [ DefaultMaterial {
                diffuseColor: "blue"
            }
        ]

        //! [animation]
        SequentialAnimation on y {
            loops: Animation.Infinite
            NumberAnimation {
                duration: 3000
                to: -150
                from: 150
                easing.type:Easing.InQuad
            }
            NumberAnimation {
                duration: 3000
                to: 150
                from: -150
                easing.type:Easing.OutQuad
            }
        }
        //! [animation]
    }*/
    //! [objects]
}
