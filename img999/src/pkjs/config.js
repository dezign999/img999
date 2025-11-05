module.exports = [
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Options",
                "description": "text"
            },
            {
                "type": "color",
                "messageKey": "hourColor",
                "defaultValue": "FFFFFF",
                "label": "Hour Color",
                "sunlight": false,
                "allowGray": false
            },
            {
                "type": "color",
                "messageKey": "minuteColor",
                "defaultValue": "FFFFFF",
                "label": "Minute Color",
                "sunlight": false,
                "allowGray": false
            },
            {
                "type": "color",
                "messageKey": "gradColor",
                "defaultValue": "000000",
                "label": "Overlay Color",
                "sunlight": false,
                "allowGray": true
            },
            {
                "type": "input",
                "messageKey": "url",
                "label": "Image URL",
                "attributes": {
                    "placeholder": "Link to a JPG or PNG image",
                    "limit": 300,
                    "type": "input"
                }
            },
            {
                "type": "button",
                "primary": false,
                "id": "clearButton",
                "defaultValue": "Clear Url"
            },
            {
                "type": "text",
                "defaultValue": "Add a \"Direct Link\" to an online image. Host your images for Free at Postimages.org. Create an account to save your image library.<br><a style=\"color:#ff4700;\" href=\"https://postimages.org\">Click Here > Postimages.org</a><div style=\"clear:both\"></div>"

            },
            {
                "type": "select",
                "messageKey": "gradient",
                "capabilities": ["COLOR"],
                "defaultValue": "0",
                "label": "Overlay",
                "description": "Add an overlay to the image to help with visibility.",
                "options": [
                    {
                        "label": "None",
                        "value": "0"
                    },
                    {
                        "label": "Solid",
                        "value": "1"
                    },
                    {
                        "label": "Checkerboard",
                        "value": "2"
                    },
                    {
                        "label": "Screen Door",
                        "value": "3"
                    }
                ]
            },
            {
                "type": "select",
                "messageKey": "gradient",
                "capabilities": ["BW"],
                "defaultValue": "0",
                "label": "Overlay",
                "description": "Add an overlay to the image to help with visibility.",
                "options": [
                    {
                        "label": "None",
                        "value": "0"
                    },
                    {
                        "label": "Solid",
                        "value": "1"
                    },
                    {
                        "label": "Black Checkerboard",
                        "value": "2"
                    },
                    {
                        "label": "White Checkerboard",
                        "value": "3"
                    }
                ]
            },
            {
                "type": "select",
                "messageKey": "layout",
                "defaultValue": "0",
                "label": "Time Position:",
                "options": [
                    {
                        "label": "Left",
                        "value": "0"
                    },
                    {
                        "label": "Right",
                        "value": "1"
                    },
                    {
                        "label": "Top",
                        "value": "2"
                    },
                    {
                        "label": "Bottom",
                        "value": "3"
                    }
                ]
            },
            {
                "type": "select",
                "messageKey": "comp1",
                "defaultValue": "1",
                "label": "Complication 1:",
                "options": [
                    {
                        "label": "None",
                        "value": "0"
                    },
                    {
                        "label": "Day",
                        "value": "1"
                    },
                    {
                        "label": "Date",
                        "value": "2"
                    },
                    {
                        "label": "Month",
                        "value": "4"
                    },
                    {
                        "label": "Small Day & Date",
                        "value": "3"
                    },
                    {
                        "label": "Month|Date",
                        "value": "5"
                    },
                    {
                        "label": "Steps",
                        "value": "6"
                    }
                ]
            },
            {
                "type": "select",
                "messageKey": "comp2",
                "defaultValue": "2",
                "label": "Complication 2:",
                "options": [
                    {
                        "label": "None",
                        "value": "0"
                    },
                    {
                        "label": "Day",
                        "value": "1"
                    },
                    {
                        "label": "Date",
                        "value": "2"
                    },
                    {
                        "label": "Month",
                        "value": "4"
                    },
                    {
                        "label": "Small Day & Date",
                        "value": "3"
                    },
                    {
                        "label": "Month|Date",
                        "value": "5"
                    },
                    {
                        "label": "Steps",
                        "value": "6"
                    }
                ]
            },
			{
			  "type": "select",
			  "messageKey": "hourlyVibe",
			  "defaultValue": "0",
			  "label": "Time Vibration",
			  "description": "Vibrate every hour or quarter hour",
			  "options": [
			    {
			      "label": "Never",
			      "value": "0"
			    },
			    {
			      "label": "Every Quarter Hour",
			      "value": "1"
			    },
			    {
			      "label": "Every Hour",
			      "value": "2"
			    }
			  ]
			},
            {
				"type": "toggle",
				"messageKey": "vibeStrong",
				"label": "Vibration Strength",
				"description": "Off for a gentle vibe, On for a strong vibe",
				"defaultValue": false,
			},
			{
				"type": "slider",
				"messageKey": "startTime",
				"defaultValue": 7,
				"label": "Watch Face Start Time",
				"description": "Set the hour the watch face becomes active (e.g., 7 = 7 AM)",
				"min": 0,
				"max": 23,
				"step": 1
			},
			{
				"type": "slider",
				"messageKey": "endTime",
				"defaultValue": 22,
				"label": "Watch Face End Time",
				"description": "Set the hour the watch face becomes inactive (e.g., 22 = 10 PM)",
				"min": 0,
				"max": 23,
				"step": 1
			},
            {
                "type": "submit",
                "defaultValue": "Save Settings"
            }
        ]
    },
            {
                "type": "heading",
                "defaultValue": "Theme Share",
                "size": 4
            },
            { 
                "type": "text",
                "defaultValue": "Copy the code below to share your theme with others. Just paste this code into the URL field and click Save to activate! Make sure your codes end in \"999\" to work correctly.",
            },
            { 
                "type": "text",
                "capabilities": ["BW"],
                "defaultValue": "<b>Black & White Pebbles:</b> When using a Color theme code, you will need to <u>adjust your color settings</u>. There is basic logic in place to handle this but it's not perfect.",
            },
            {
                "type": "section",
                "items": [
                    { 
                        "type": "text",
                        "id" : "themeCode",
                        "defaultValue": "",
                    }
                ]
              }
			
   

    
];