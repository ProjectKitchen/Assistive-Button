const char MAIN_page[] PROGMEM = R"=====(
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <meta http-equiv="Content-Script-Type" content="text/javascript" /> 
    <meta http-equiv="Content-Style-Type" content="text/css" />
    <meta http-equiv="content-language" content="en" />
     
    <meta http-equiv="pragma" content="no-cache" />
    <meta http-equiv="cache-control" content="no-cache" />
    <meta http-equiv="expires" content="0" />
       
    <meta http-equiv="imagetoolbar" content="no" />
    <meta name="slice" scheme="IE" content="off" />

    <meta name='viewport' content='width=device-width, initial-scale=1'>

    <title>Assistive Button Demo</title>

    <style>
      * { padding: 0; margin: 0; vertical-align: top; }
      body { background: url(background.png) repeat-x; font: 18px/1.5em "proxima-nova", Helvetica, Arial, sans-serif; }
      h2 { margin-top: 15px; font: normal 22px "omnes-pro", Helvetica, Arial, sans-serif; }
      p { margin-top: 10px; }
      input[type="radio"] { margin: 6; }
      input[type="button"] { width: 195px; font-size: 18px; padding: 2px 5px; }
      select { width: 195px; font-size: 18px; padding: 1px 5px; }
      #header { position: relative; width: 850px; margin: auto; }
      #header h2 { margin-left: 0px; vertical-align: middle; font-size: 28px; font-weight: bold; text-decoration: none; color: #000; }
      #content { width: 850px; margin: 0 auto; padding: 10px; }
      #footer { margin-top: 25px; margin-bottom: 10px; text-align: center; font-size: 12px; color: #999; }
    </style>
  </head>

  <body>
    <div id="header">
      <h2>Welcome to the Assistive Button Interface</h2>
    </div>
    
    <div id="content">
      <form id="modeSelection" action="mode.html" method="post" encytpe="text/plain">
        <h2>Please select a mode to configure the device and hit the "Send Mode Selection" Button</h2>
        <p>
          <div id="modes">
            <input type="radio" name="mode" value="playMode" id="playMode">Use play mode<br>
            <input type="radio" name="mode" value="recordMode" id="recordMode">Use record mode<br>
          </div>
        </p>
        <p>
          <input type="button" value=" Send Mode Selection " id="modeSelectionBtn" name="modeSelectionBtn" onclick="sendMode()" />
        </p>
      </form>

      <form id="songSelection" action="song.html" method="post" encytpe="text/plain">
        <h2>Choose a File for playmode or instant replay</h2>
        <p>
          <select id="songSelect">
            <optgroup label="Songs" id="songs"></optgroup>
            <optgroup label="Audiobooks" id="audiobooks"></optgroup>
            <optgroup label="Records" id="records"></optgroup>
          </select>
        </p>
        <p>
          <input type="button" value=" Get playlist" id="getPL" name="getPL" onclick="getPlaylist()" />
          <input type="button" value=" Load selected File " id="loadBtn" name="loadBtn" onclick="loadFile()" />
          <input type="button" value=" Play selected File " id="playBtn" name="playBtn" onclick="playFile()" />
        </p>
      </form>
    </div>

    <div id="footer">
      Gregor Maier - sa15b026@technikum-wien.at
    </div>    

    <script type="text/javascript">
  
    function removeOptions(selectbox) {
        while (selectbox.firstChild) {
          selectbox.removeChild(selectbox.firstChild);
        }
    }
    
    function updateOpt(options) {
      removeOptions(document.getElementById("songs"));
      removeOptions(document.getElementById("audiobooks"));
      removeOptions(document.getElementById("records"));
    
      var splitted = new Array();
      splitted = options.split(",");
      
      var i = 0;
      var select;
      var opts = new Array();
      
      for (a in splitted) {
        opts = [];
        var single = new Array();
        single = splitted[a].split("|");
        for (s in single) {
            switch(i) {
              case 0:
                  select = document.getElementById("songs");
                    opts.push(single[s]);
                    break;
                case 1:
                  select = document.getElementById("audiobooks");
                    opts.push(single[s]);
                    break;
                case 2:
                  select = document.getElementById("records");
                    opts.push(single[s]);
                    break;
            }
        }
        for(var j = 0; j < opts.length; j++) {
          var opt = opts[j];
          var el = document.createElement("option");
          el.textContent = opt;
          el.value = opt;
          select.appendChild(el);
        }
        i++;
      }      
    }

    function sendMode() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          //document.getElementById("modes").innerHTML = this.responseText;
          alert(this.responseText);
        }
      };
      var selected;
      if (document.getElementById('playMode').checked) {
        selected = document.getElementById('playMode').value;
      } else if (document.getElementById('recordMode').checked) {
        selected = document.getElementById('recordMode').value;
      }
      
      xhttp.open("GET", "mode?mode="+selected, true);
      xhttp.send();
      console.log("mode?mode="+selected);
    }

    function loadFile() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          //document.getElementById("loadBtn").innerHTML = this.responseText;
          alert(this.responseText);
        }
      };
      xhttp.open("GET", "load?song="+document.getElementById('songSelect').value, true);
      xhttp.send();
      console.log("load?song="+document.getElementById('songSelect').value);
    }

    function playFile() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          //document.getElementById("playBtn").innerHTML = this.responseText;
          alert(this.responseText);
        }
      };
      xhttp.open("GET", "play?song="+document.getElementById('songSelect').value, true);
      xhttp.send();
      console.log("play?song="+document.getElementById('songSelect').value);
    }

    function getPlaylist() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.responseText && this.responseText.length > 0) {
          console.log(this.responseText);
          updateOpt(this.responseText);
        }
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("getPL").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "getList", true);
      xhttp.send();
    }
    
    </script>
  </body>
</html>
)=====";
