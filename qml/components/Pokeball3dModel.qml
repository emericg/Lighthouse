import QtQuick3D

Node {
    id: control

    Node {
        id: pokeball

        ////////

        position: Qt.vector3d(0, 0, 0)
        scale: Qt.vector3d(1, 1, 1)
        eulerRotation: Qt.vector3d(66, 0, -180)

        ////////

        PrincipledMaterial {
            id: mat_material_white
            baseColor: "#fff"
            metalness: 0.1
            roughness: 0.66
            cullMode: Material.NoCulling
        }
        PrincipledMaterial {
            id: mat_material_black
            baseColor: "#ff0a0a0a"
            metalness: 0.1
            roughness: 0.3
            cullMode: Material.NoCulling
        }
        PrincipledMaterial {
            id: mat_material_white2
            baseColor: "#fff"
            metalness: 0.1
            roughness: 0.3
            cullMode: Material.NoCulling
        }
        PrincipledMaterial {
            id: mat_material_white3
            baseColor: "#ddd"
            metalness: 0.1
            roughness: 0.3
            cullMode: Material.NoCulling
        }
        PrincipledMaterial {
            id: mat_material_red
            baseColor: "#ffce0000"
            metalness: 0.1
            roughness: 0.66
            cullMode: Material.NoCulling
        }

        ////////

        Model {
            id: bottom_
            y: -0.584008
            rotation: Qt.quaternion(-7.54979e-08, 0, 1, 0)
            source: "qrc:/assets/models/pokeball_meshes/bottom_.mesh"

            materials: [
                mat_material_white
            ]

            Model {
                id: bottom_black
                rotation: Qt.quaternion(5.33851e-08, 5.0243e-15, 0.707107, 0.707107)
                scale.x: 1
                scale.y: 1
                scale.z: 1
                source: "qrc:/assets/models/pokeball_meshes/bottom_black.mesh"

                materials: [
                    mat_material_black
                ]
            }
        }

        ////////

        Model {
            id: top_black
            y: 99.416
            rotation: Qt.quaternion(0.707107, -0.707107, 0, 0)
            scale.y: 1
            scale.z: 1
            source: "qrc:/assets/models/pokeball_meshes/top_black.mesh"

            materials: [
                mat_material_black
            ]

            Model {
                id: button
                z: -200
                source: "qrc:/assets/models/pokeball_meshes/button.mesh"

                x: pokeballView.joyx
                y: pokeballView.joyy
                materials: [
                    mat_material_white3
                ]
            }

            Model {
                id: button_holder
                z: -197.662
                source: "qrc:/assets/models/pokeball_meshes/button_holder.mesh"

                materials: [
                    mat_material_white2
                ]
            }

            Model {
                id: top_red
                z: -100
                rotation: Qt.quaternion(0.707107, 0.707107, 0, 0)
                scale.y: 1
                scale.z: 1
                source: "qrc:/assets/models/pokeball_meshes/top_red.mesh"

                materials: [
                    mat_material_red
                ]
            }
        }

        ////////
    }
}
