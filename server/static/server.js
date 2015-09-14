/**
 * Created by tj on 14/07/15.
 */

      var PLAY_WIDTH = 600;
      var PLAY_HEIGHT = 400;
      var PlayFactorX;
      var PlayFactorY;
      var PlayMarginX;
      var PlayMarginY;


      var circle;
      var layer;
      var myIp = "192.168.31.134:8080";
      var socket = new WebSocket("ws://" + myIp +"/ws");

      socket.onopen = function(){
        console.log("connected");
      };

      socket.onmessage = function (message) {
        console.log("receivingZZZZsssZZZZZZ: " + message.data);
        if (isNumeric(message.data)) {
          var x = parseInt(message.data, 10);
          var cXY = circle.getPosition();
             console.log("receivingZZZZZZZZZZ: New X " + x + " Circle X: " + cXY.x );
          circle.move(x - cXY.x, 0);
          layer.draw();
        }
        else
          {
            try {

               var js = JSON.parse(message.data);

                switch(js.id){
                    case 'T2':
                        console.log("TEMPAREATURE >>--->> " + js.T + " HUMIDITY: " + js.H  );
                        logger (js.T,js.H ) ;
                        break;
                    case 'Z3':
                        console.log("PAN TILT POSITION ---> X:" + js.x + " Y: " + js.y  + " XF " + PlayFactorX );
                        moveCircle(js.x ,js.y );
                        break;
                    case 'Z5': // ' receive initial coordinate'
                        PlayFactorX = PLAY_WIDTH/(js.xmax-js.xmin);
                        PlayFactorY = PLAY_HEIGHT/(js.ymax-js.ymin);
                        PlayMarginX = js.xmin;
                        PlayMarginY = js.ymin;
                        moveCircle(js.x ,js.y );
                        break;
                    case 'XZ':
                        //http://stackoverflow.com/questions/9991805/javascript-how-to-parse-json-array
                        console.log("PATROL =======");
                        for (var i  = 0; i < js.patrol.length; i++) {
                            var point = js.patrol[i];
                            console.log(point.x + "   " + point.y + "   " + point.d );
                            plotPatrol(point.x ,point.y );
                        }


                        break;
                    default :
                        console.log("UNKNOPWN HEADER " + js.id );

                        break;
                }
            }
            catch(err){
              console.log("NOT XXXXQQQQ " + err);
            }


          }

      };

      socket.onclose = function(){
        console.log("disconnected");
      };

      sendMessage = function(message) {
        socket.send(message);
      };

      var value = 0;

      window.onload = function() {
        var stage = new Kinetic.Stage({
          container: "container",
          width: PLAY_WIDTH,
          height: PLAY_HEIGHT
        });





        layerPatrol  = new Kinetic.Layer();
         // layerPatrol.hide();
        layer  = new Kinetic.Layer();

        circle = new Kinetic.Circle({
          x: 100,
          y: 100,
          radius: 10,
          fill: "red",
          stroke: "black",
          strokeWidth: 4,
          draggable: true
        });



        circle.on("mousedown", function() {
          value = value + 20;
          console.log('sending: ' + value);
          sendMessage("{value:" + value + "}");
        });

        layer.add(circle);

        stage.add(layerPatrol);
        stage.add(layer);

        enableStageClick();

        function enableStageClick() {
            $(stage.getContent()).on('click', function (event) {
                var pos = stage.getMousePosition();
                console.log( "SSSSSSSSX " + pos.x + " PlayFactorX =" + PlayFactorX + "  FINALX:" + parseInt(pos.x / PlayFactorX) );
                console.log( "SSSSSSSSY " + pos.y );
                var mouseX = parseInt(PlayMarginX) + (pos.x / PlayFactorX);
                var mouseY = parseInt(PlayMarginY) + (pos.y / PlayFactorY);
                var cXY = circle.getPosition();

                var obj = new Object();
                obj.id = "C1";
                obj.x = parseInt(mouseX).toString();
                obj.y =  parseInt(mouseY).toString();
                obj.sender = 0;

                var HitString = JSON.stringify(obj);

                sendMessage(HitString);
                //circle.move(mouseX - cXY.x , mouseY - cXY.y);
                //layer.draw();
                //var hit = rect1.hit(mouseX, mouseY) ? "You clicked inside rect" : "You clicked outside rect";
                //$("#hit").text(hit);
            });
        }


      };

      function isNumeric(n) {
         return !isNaN(parseFloat(n)) && isFinite(n);
          }

      function moveCircle(x,y){
          var cXY = circle.getPosition();
          circle.move(((x-PlayMarginX) * PlayFactorX) - cXY.x, ((y - PlayMarginY) * PlayFactorY) - cXY.y);
          layer.draw();
      }

function plotPatrol(px,py){

        circleZ = new Kinetic.Rect({
          x: px,
          y: py,
          width: 10,
          height: 10,
          fill: "blue",
          stroke: "black",
          strokeWidth: 2,
          draggable: false
        });
        layerPatrol.add(circleZ);
        layerPatrol.draw();

}

        var logger = function(msg,msgH){
          var now = new Date();
          var sec = now.getSeconds();
          var min = now.getMinutes();
          var hr = now.getHours();
        //  $("#log").html($("#log").html() + "<br/>" + hr + ":" + min + ":" + sec + " ___ " +  msg);

            ShowTemp(msg,msgH);
          //$("#log").animate({ scrollTop: $('#log')[0].scrollHeight}, 100);
        //  $('#log').scrollTop($('#log')[0].scrollHeight);
        }


  function ShowTemp(temp1,temp2)
	{
      var ds18b20_reading = document.getElementById('temp1_value');
      var cpu_reading = document.getElementById('temp2_value');
      ds18b20_reading.innerHTML = parseFloat(temp1).toFixed(2);
      cpu_reading.innerHTML = parseFloat(temp2).toFixed(2);
      drawGraph_environment(temp1);
      //drawGraph_CPU(temp2);
	}

  function drawGraph_environment(temp1){
    var canvas = document.getElementById('cvs_E');
    RGraph.Clear(canvas);

    var thermometer = new RGraph.Thermometer('cvs_E', 0,60,     eval(temp1))
    .set('title.side', 'Environment')
    .set('value.label', false)
    .set('scale.visible', true)
    .set('scale.decimals',2)
    .set('gutter.left', 25)
    .set('gutter.right', 40)
    .draw();
  }

    function get_ip(val){
        myIp=val;

    }