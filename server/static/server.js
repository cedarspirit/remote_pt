/**
 * Created by tj on 14/07/15.
 */


      var circle;
      var layer;

      var socket = new WebSocket("ws://192.168.31.130:8080/ws");

      socket.onopen = function(){
        console.log("connected");
      };

      socket.onmessage = function (message) {
        console.log("receivingZZZZZZZZZZ: " + message.data);
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
                        console.log("TEMPAREATURE ---> " + js.T + " HUMIDITY: " + js.H  );
                        logger (js.T) ;
                        break;
                    case 'Z3':
                        console.log("PAN TILT POSITION ---> X:" + js.x + " Y: " + js.y  );
                        moveCircle(js.x,js.y);
                        break;
                    default :
                        console.log("UNKNOPWN HEADER " + js.id )
                        break;
                }
            }
            catch(err){
              console.log("NOT XXXX " + err.message);
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
          width: 600,
          height: 400
        });




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
        stage.add(layer);

        enableStageClick();

        function enableStageClick() {
            $(stage.getContent()).on('click', function (event) {
                var pos = stage.getMousePosition();
                var mouseX = parseInt(pos.x);
                var mouseY = parseInt(pos.y);
                var cXY = circle.getPosition();


                var HitObject = {
                      "id": "D1",
                      "x": mouseX,
                      "y": mouseY
                };
                var HitString = JSON.stringify(HitObject);

                sendMessage(HitString);
                circle.move(mouseX - cXY.x , mouseY - cXY.y);
                layer.draw();
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
          circle.move(x - cXY.x, y - cXY.y);
          layer.draw();
      }

        var logger = function(msg){
          var now = new Date();
          var sec = now.getSeconds();
          var min = now.getMinutes();
          var hr = now.getHours();
        //  $("#log").html($("#log").html() + "<br/>" + hr + ":" + min + ":" + sec + " ___ " +  msg);

            ShowTemp(msg,msg);
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
      drawGraph_CPU(temp2);
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