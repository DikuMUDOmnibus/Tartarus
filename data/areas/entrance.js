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
            "exits": [-1, 2, 0, 3],
            "exit_areas": [0, 0, 0, 0],
            "objects": [
                {
                    "name": "Nail Bat",
                    "is_static": 0,
                    "rarity": 1,
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
            "exits": [5, -1, -1, 1],
            "exit_areas": [0, 0, 0, 0],
            "objects": [
                {
                    "name": "Leather Boots",
                    "is_static": 0,
                    "rarity": 0,
                    "type": 1,
                    "armor": 10,
                    "wear_location": 1024,
                    "keywords": ["leather", "boots", "leather boots"]
                },
                {
                    "name": "Sandals",
                    "is_static": 0,
                    "rarity": 0,
                    "type": 1,
                    "armor": 5,
                    "wear_location": 1024,
                    "keywords": ["sandals"]
                }
            ]
        },
        {
            "id": 3,
            "area_id": 0,
            "name": "Cafeteria",
            "description": "Lesser demons sit around circular tables while dining\non human meat. The humans are being eaten alive and made\nto watch as the demons devour their raw flesh.",
            "exits": [-1, 1, -1, 4],
            "exit_area": [0, 0, 0, 0],
            "objects": [
                {
                    "name": "Rusty Steak Knife",
                    "is_static": 0,
                    "rarity": 0,
                    "type": 0,
                    "armor": 0,
                    "keywords": ["knife", "rusty", "steak", "steak knife", "rusty steak knife"]
                }
            ]
        },
        {
            "id": 4,
            "area_id": 0,
            "name": "Kitchen",
            "description": "Servant demons prepare different meals for the lesser\ndemons to consume. Some of the choice foods are Boiled\nHands Soup, Raw Thigh Steaks, and even steamed eyeballs.",
            "exits": [-1, 3, -1, -1],
            "exit_area": [0, 0, 0, 0],
            "objects": []
        },
        {
            "id": 5,
            "area_id": 0,
            "name": "Hall of Blades",
            "description": "As you walk farther from the torture chamber, the screams\nare now muffled. Empty suits of demonic armor line the\nwalls. As you make your way through the hall, you can't\nhelp but feel like you're being watched.",
            "exits": [6, -1, 2, -1],
            "exit_area": [0, 0, 0, 0],
            "objects": []
        },
        {
            "id": 6,
            "area_id": 0,
            "name": "Hall of Blades",
            "description": "This section of the hall is adorned with swords hanging from\nthe walls. It might be possible to rip one off the wall...",
            "exits": [7, -1, 5, -1],
            "exit_area": [0, 0, 0, 0],
            "objects": [
                {
                    "name": "Bronze Short Sword",
                    "is_static": 0,
                    "rarity": 0,
                    "type": 0,
                    "armor": 0,
                    "keywords": ["bronze", "short", "sword", "short sword", "bronze sword", "bronze short sword"]
                }
            ]
        },
        {
            "id": 7,
            "area_id": 0,
            "name": "Sleeping Quarters",
            "description": "One of the lesser demons is asleep on a bed of hot coals.\nYou should try killing it while it's still sleeping.",
            "exits": [-1, -1, 6, -1],
            "exit_area": [0, 0, 0, 0],
            "objects": []
        }
    ]
}
