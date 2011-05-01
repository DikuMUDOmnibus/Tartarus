{
    "id": 0,
    "name": "Prison of Souls",
    "rooms": [
        {
            "id": 0,
            "area_id": 0,
            "name": "Entrance to Tartarus",
            "description": "The entrance to Tartarus is filled with the souls of those who\ncommitted horrible crimes on Earth. They are here waiting for\nadmission into a hell of their own creation. The way out of\nTartarus is blocked by a vast river of fire.",
            "exits": [1, -1, -1, -1],
            "exit_areas": [0, 0, 0, 0],
            "objects": []
        },
        {
            "id": 1,
            "area_id": 0,
            "name": "Torture Chamber",
            "description": "The walls are painted in fresh, warm blood. Demons scurry about\nin butcher's aprons covered in human blood. You hear the\nscreams of people being torn apart by various instruments of pain.\nTheir bodies have been reanimated to feel excruciating death over and over.",
            "exits": [-1, 2, 0, -1],
            "exit_areas": [0, 0, 0, 0],
            "objects": [
                {
                    "name": "Nail Bat",
                    "is_static": 0,
                    "rarity": 2,
                    "type": 0,
                    "armor": 0,
                    "keywords": ["nail", "bat", "nail bat"]
                }
            ]
        },
        {
            "id": 2,
            "area_id": 0,
            "name": "Fire Pits",
            "description": "You can hear the echoes of screams from the Torture Chamber\nnext door. You see vast pits of fire filled with the souls\nof arsons brought here to suffer the pain of death by fire.",
            "exits": [-1, -1, -1, 1],
            "exit_areas": [0, 0, 0, 0],
            "objects": [
                {
                    "name": "Leather Boots",
                    "is_static": 0,
                    "rarity": 1,
                    "type": 1,
                    "armor": 10,
                    "wear_location": 1024,
                    "keywords": ["leather", "boots", "leather boots"]
                },
                {
                    "name": "Sandals",
                    "is_static": 0,
                    "rarity": 1,
                    "type": 1,
                    "armor": 5,
                    "wear_location": 1024,
                    "keywords": ["sandals"]
                }
            ]
        }
    ]
}
