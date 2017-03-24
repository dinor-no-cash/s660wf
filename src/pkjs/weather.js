var xhrRequest = function (url, type, callback){
	var xhr = new XMLHttpRequest();
	xhr.onload = function(){callback(this.responseText);};
	xhr.open(type, url);
	xhr.send();
};

function locationSuccess(pos){
	var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' + pos.coords.latitude + '&lon=' + pos.coords.longitude + '&units=metric&appid=4e387170dd73515d3bbc15e5073484bd';
	xhrRequest(url, 'GET', 
		function(responseText){
			var json = JSON.parse(responseText);
			var temperature = Math.round(json.main.temp);
			var humidity = Math.round(json.main.humidity);
			var dictionary = {'TEMPERATURE': temperature, 'HUMIDITY': humidity};
			Pebble.sendAppMessage(dictionary, function(e){}, function(e){});
		}
	);
}
function locationError(err){}

function getWeather(){
	navigator.geolocation.getCurrentPosition(locationSuccess, locationError, {timeout: 15000, maximumAge: 60000});
}

Pebble.addEventListener('ready', function(e){getWeather();});
Pebble.addEventListener('appmessage', function(e){getWeather();});
