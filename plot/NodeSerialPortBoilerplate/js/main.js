/**
 * Created by ghassaei on 10/26/16.
 */


$(function() {

    var socket = io.connect('http://localhost:8080');
    var serialMonitor = document.getElementById("serialMonitor");
    var serialMonitor2 = document.getElementById("serialMonitor2");
    var stringBuffer1 = new Array(100);

    $("#refreshPorts").click(function(e){
        e.preventDefault();
        socket.emit("refreshPorts");
    });

    //bind events
    socket.on('connected', function(data){

        console.log("connected");

        if (data.portName) $("#portName").html(data.portName);
        if (data.baudRate) $("#baudRate").html(data.baudRate);
        if (data.availablePorts && data.availablePorts.length>0){
            var html = "";
            for (var i=0;i<data.availablePorts.length;i++){
                html += "<a class='availablePorts' href='#'>" + data.availablePorts[i] + "</a><br/>"
            }
            $("#availablePorts").html(html);
        }

        $(".availablePorts").click(function(e){//change port
            e.preventDefault();
            socket.emit("portName", $(e.target).html());
        });
    });

    socket.on('portConnected', function(data){
        console.log("connected port " + data.portName + " at " + data.baudRate);
    });

    socket.on('portDisconnected', function(data){
        console.log("disconnected port " + data.portName + " at " + data.baudRate);
    });

    socket.on("errorMsg", function(data){
        console.warn(data);
    });

    socket.on("error", function(error){
        console.warn(error);
    });

    socket.on("connect_error", function(){
        console.log("connect error");
    });

    socket.on("dataIn", function(data){//oncoming serial data
        split_data = data.split(" ");
        // console.log(split_data)
        if (split_data[0] == "N1") {
        // console.log("data: " + data);
            // $("#dataDisplay").html(split_data[3]);
            stringBuffer1.pop();
            stringBuffer1.unshift(split_data[3] + " " + split_data[5] + "<br>");
            console.log(stringBuffer1)
            // stringBuffer1.pop();
            serialMonitor.innerHTML = stringBuffer1;
            // dataStream.append(new Date().getTime(), split_data[3].split(",")[0]);
        } else if (split_data[0] == "N2") {
            // serialMonitor2.innerHTML = split_data[3] + "<br>" + serialMonitor2.innerHTML;
        }
    });

});
