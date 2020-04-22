#ifndef _esp8266html.h    
#define _esp8266html.h    

const char index_html[] PROGMEM={
"<!DOCTYPE html>\n"
"<html lang=\"en\">   \n"
"<head>\n"
"    <meta charset=\"utf-8\"/>\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/>\n"
"    <title>My Button</title>\n"
"    <style>\n"
"        body{\n"
"            font-family: Arial;\n"
"        }\n"
"        h1{\n"
"            text-align: center; margin: 0 auto; padding: 20px; font-size: 30px; \n"
"        }\n"
"        p{\n"
"            text-align: center; margin-top: 30px; font-weight: bold; font-size: 15px;\n"
"        }\n"
"        .well{\n"
"            height: 50px; width: 60%; margin: 0 auto; padding-top: 20px; font-size: 18px; font-weight: bold;\n"
"            border-style: ridge; border-width: thin;\n"
"        }\n"
"        .submitHolder{\n"
"            text-align: center; margin-top: 10px;\n"
"        }\n"
"        .playlistHolder{\n"
"            text-align: center; margin-top: 40px;\n"
"        }\n"
"        input[type=submit]{\n"
"            cursor: pointer; background-color: rgb(204, 203, 203); font-size: large; font-weight: bold;\n"
"        }\n"
"        input[type=submit]:hover{\n"
"            background-color: rgb(151, 147, 147);\n"
"        }\n"
"        #mySelect{\n"
"            font-size: large; width:fit-content; background-color: rgb(248, 248, 245); border-style: outset; border-color: rgb(192, 190, 190);\n"
"        }\n"
"    </style>\n"
"</head>\n"
"\n"
"<body>\n"
"    <h1>Please select mode and push submit!</h1>\n"
"   \n"
"              <form style=\"margin-top: 20px;\">\n"
"                <div class=\"well\"; style=\"text-align: left; border-color: rgb(187, 49, 49); background-color: rgb(241, 211, 211);\">\n"
"                    <input id=\"Record Modus\" input type=\"radio\" name=\"mode\" value=\"0\">Record<br>\n"
"                </div>\n"
"                   \n"
"                <div class=\"well\"; style=\"text-align: left; border-color: rgb(5, 136, 23); background-color: rgb(224, 241, 224);\">\n"
"                    <input id=\"Play recorded\" input type=\"radio\" name=\"mode\" value=\"1\">Play recorded file<br>\n"
"                </div>\n"
"                   \n"
"                <div class=\"well\"; style=\"text-align: left; border-color: rgb(26, 71, 196);background-color: rgb(210, 218, 243);\">\n"
"                    <input id=\"Playlist Modus\" input type=\"radio\" name=\"mode\" value=\"2\">Play stored file<br>\n"
"                </div>\n"
"            </form>\n"
"        <div class=\"submitHolder\">\n"
"            <input style=\"padding: 7px;\" type=\"submit\" id=\"submit\" value=\"Submit\"/>\n"
"        </div>\n"
"        <div>\n"
"            <p>Submitting \"Play stored file\" will load the file names.</p>\n"
"        </div>\n"
"        <div class=\"playlistHolder\">\n"
"        <select id=\"mySelect\">\n"
"            <option value=\"\" disabled selected>Select a file</option>\n"
"        </select>\n"
"        <input style=\"margin-left: 30px;\" id=\"confirm\" type=\"submit\"; value=\"Confirm Song\";/> \n"
"        </div>\n"
"    <br>\n"
"    \n"
"    <script> type=\"text/javascript\"\n"
"        var modus=\"\";\n"
"        var selectedSong=\"\";\n"
"        //var myArray = new Array(\"Denis\", \"Steffie\", \"Hans\", \"Doris\");\n"
"        //var namen=\"\";\n"
"        var namenarray=new Array();\n"
"        \n"
"        document.getElementById(\"Record Modus\").onclick = function(){\n"
"                var ident= document.getElementById(\"mySelect\");\n"
"                while (ident.hasChildNodes()) {\n"
"                ident.removeChild(ident.lastChild);} }\n"
"        document.getElementById(\"Play recorded\").onclick = function(){\n"
"                var ident= document.getElementById(\"mySelect\");\n"
"                while (ident.hasChildNodes()) {\n"
"                ident.removeChild(ident.lastChild);} }\n"
"\n"
"               \n"
"        document.getElementById(\"submit\").onclick = function(){\n"
"            if(document.getElementById(\"Record Modus\").checked){\n"
"                var ident= document.getElementById(\"mySelect\");\n"
"                while (ident.hasChildNodes()) {\n"
"                ident.removeChild(ident.lastChild);}\n"
"                modus=document.getElementById(\"Record Modus\").value;\n"
"                sendMode();\n"
"            }\n"
"\n"
"            if(document.getElementById(\"Play recorded\").checked){\n"
"                var ident= document.getElementById(\"mySelect\");\n"
"                while (ident.hasChildNodes()) {\n"
"                ident.removeChild(ident.lastChild);}\n"
"                modus=document.getElementById(\"Play recorded\").value;\n"
"                sendMode(); \n"
"            }\n"
"\n"
"            if(document.getElementById(\"Playlist Modus\").checked){\n"
"            //alert(\"es funktioniert\");\n"
"                modus=document.getElementById(\"Playlist Modus\").value;\n"
"                sendMode();\n"
"                loadList();\n"
"                                                          \n"
"                //var confirmsong=document.getElementById(\"confirm\");\n"
"                //confirmsong.type=\"submit\";\n"
"                //confirmsong.value=\"Confirm Song\";\n"
"                                              \n"
"            }\n"
"        }\n"
"        document.getElementById(\"confirm\").onclick = function(){\n"
"        var selOption = document.getElementById(\"mySelect\");\n"
"        selectedSong = selOption.options[selOption.selectedIndex].value;\n"
"        //alert(selectedSong)\n"
"        sendSelected();\n"
"        }\n"
"\n"
"        function loadList() {\n"
"        var xhttp = new XMLHttpRequest();\n"
"        xhttp.onreadystatechange = function() {\n"
"            if (this.readyState == 4 && this.status == 200) {\n"
"            //alert(this.responseText);\n"
"            namenarray = (this.responseText).split(\",\");\n"
"            //alert(namenarray);\n"
"                var ident= document.getElementById(\"mySelect\");\n"
"                while (ident.hasChildNodes()) {\n"
"                ident.removeChild(ident.lastChild);}\n"
"                for(i=0;i<namenarray.length;i++){\n"
"                //var song= namenarray[i];\n"
"                var x = document.createElement(\"OPTION\");\n"
"                x.textContent = namenarray[i];\n"
"                x.value = x.textContent;\n"
"                ident.appendChild(x);} \n"
"                }\n"
"            };\n"
"            xhttp.open(\"GET\", \"/playlist\", true);\n"
"            xhttp.send();\n"
"            \n"
"            //alert(namen);\n"
"            //namenarray=namen.split(\",\");\n"
"            \n"
"        }\n"
"    \n"
"        function sendMode(){\n"
"            var xhr = new XMLHttpRequest();\n"
"            xhr.onreadystatechange = function() {\n"
"            if (this.readyState == 4 && this.status < 300) {\n"
"                //alert(this.responseText);\n"
"            }\n"
"            };\n"
"            xhr.open(\"POST\", \"/mode\", true);\n"
"            xhr.setRequestHeader(\"Content-Type\", \"text/plain\")\n"
"            xhr.send(modus);\n"
"        }\n"
"        function sendSelected(){\n"
"            var xhr1 = new XMLHttpRequest();\n"
"            xhr1.onreadystatechange = function() {\n"
"            if (this.readyState == 4 && this.status < 300) {\n"
"                //alert(this.responseText);\n"
"            }\n"
"            };\n"
"            xhr1.open(\"POST\", \"/selected\", true);\n"
"            xhr1.setRequestHeader(\"Content-Type\", \"text/plain\")\n"
"            xhr1.send(selectedSong);\n"
"        }\n"
"\n"
"        </script>\n"
"</body>\n"
"</html>"
};   //use of progmem keyword to store the array in flash/program memory (instead of SRAM)

#endif // _esp8266html.h    // Put this line at the end of your file.
