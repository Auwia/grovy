var grovyApp = angular.module('grovyApp', ['AngularChart'], function($routeProvider, $locationProvider){
  $routeProvider.when('/', {
    template: '<chart title="-.- Grovy Chart Measurement -.-" xData="lineChartXDataLeft" yData="lineChartYDataLeft" xName="Month" yName="Hit" subtitle="Starting prototype experiment. Massimo Manganiello © 2017-09-26 Left"></chart> <chart xData="lineChartXDataRight" yData="lineChartYDataRight" xName="Month" yName="Hit" subtitle="Right"></chart> <chart xData="lineChartXDataOut" yData="lineChartYDataOut" xName="Month" yName="Hit" subtitle="Outside"></chart> <chart xData="lineChartXDataEle" yData="lineChartYDataEle" xName="Month" yName="Hit" subtitle="Electronic Measurements"></chart> <chart xData="lineChartXDataMoisture" yData="lineChartYDataMoisture" xName="Month" yName="Hit" subtitle="Moisture"></chart> <chart xData="lineChartXDataTemperature" yData="lineChartYDataTemperature" xName="Month" yName="Hit" subtitle="Temperature"></chart>'
  })
});

grovyApp.controller("GrovyCtrl", function($scope, $http){
  
  $scope.period = ["Last 15", "Day", "Month", "6 Months", "Year"];
  
  var period_default = "Day";
  $scope.selectedPeriod = "Day";
  $scope.day = moment().format('YYYY-MM-DD');

  getPhoto($scope, $http);
  getPhotos($scope, $http);
  getLight($scope, $http);
  getFanLamp($scope, $http);
  getFanIn($scope, $http);
  getFanOut($scope, $http);
  getPump($scope, $http);
  getPeltier($scope, $http);
  getLightSpectrum($scope, $http);
  getWaterLevel($scope, $http);
  getMis($scope, $http, period_default );
  getMisEle($scope, $http, period_default );
  getMisMoisture($scope, $http, period_default );
  
  setInterval(function() {
    getPhoto($scope, $http);
    // getPhotos($scope, $http);
    getLight($scope, $http);
    getFanLamp($scope, $http);
    getFanIn($scope, $http);
    getFanOut($scope, $http);
    getPump($scope, $http);
    getPeltier($scope, $http);
    getLightSpectrum($scope, $http);
    getWaterLevel($scope, $http);
    getMis($scope, $http, $scope.selectedPeriod);
    getMisEle($scope, $http, $scope.selectedPeriod);
    getMisMoisture($scope, $http, $scope.selectedPeriod);
  }, 30000);
  
  $scope.reWrite = function() {
    getMis($scope, $http, $scope.selectedPeriod);
    getMisEle($scope, $http, $scope.selectedPeriod);
    getMisMoisture($scope, $http, $scope.selectedPeriod);
  };
  
  function getMis($scope, $http, period){
    var rest_response = {};
    // Left
    var timestamps_left = []; 
    var areas_left = [];
    var temperatures_left = [];
    var humidity_left = [];
    // Right
    var timestamps_right = []; 
    var areas_right = [];
    var temperatures_right = [];
    var humidity_right = [];
    // Outside
    var timestamps_out = []; 
    var areas_out = [];
    var temperatures_out = [];
    var humidity_out = [];
    // Temperature
    var timestamps_temperature = []; 
    var areas_temperature = [];
    var temperatures_temperature = [];
    
  if (period == "Day") {
    if (isValid($scope.day)) {
      range = $scope.day;
    } else {
      range = moment().format('YYYY-MM-DD');
    }
  } else {
    range = $scope.selectedPeriod;
  }
    
    $http.get('ws/getMisPeriod.php/?period=' + range).then(function(response){
      rest_response = response.data;
      for(var i=0; i < rest_response.length; i++){
        
        // left
        if (rest_response[i]['area'] == 'Left'){
          var tmp = rest_response[i]['timestamp'];
          if ($scope.selectedPeriod == 'Last 15'){
            var lastFive = tmp.substring(11, tmp.length-3);
          }
          
          if ($scope.selectedPeriod == 'Day' || $scope.selectedPeriod == 'Month') {
            var lastFive = tmp.substr(tmp.length - 8, 5);
          } 
          
          if ($scope.selectedPeriod == 'Year'){
            var lastFive = tmp.substring(0,7);
          }
          
          if ($scope.selectedPeriod == '6 Months'){
            var lastFive = tmp.substring(0,7);
          }
          
          timestamps_left.push(lastFive);
          areas_left.push(rest_response[i]['area']);
          temperatures_left.push(rest_response[i]['temperature']);
          humidity_left.push(rest_response[i]['humidity']);
        }
        
        // right
        if (rest_response[i]['area'] == 'Right') {
          var tmp = rest_response[i]['timestamp'];
          if ($scope.selectedPeriod == 'Last 15'){
            var lastFive = tmp.substring(11, tmp.length-3);
          }
          
          if ($scope.selectedPeriod == 'Day' || $scope.selectedPeriod == 'Month') {
            var lastFive = tmp.substr(tmp.length - 8, 5);
          }  
          
          if ($scope.selectedPeriod == 'Year'){
            var lastFive = tmp.substring(0,7);
          }
          
          if ($scope.selectedPeriod == '6 Months'){
            var lastFive = tmp.substring(0,7);
          }    
          
          timestamps_right.push(lastFive);
          areas_right.push(rest_response[i]['area']);
          temperatures_right.push(rest_response[i]['temperature']);
          humidity_right.push(rest_response[i]['humidity']);
        }
        
        // outside
        if (rest_response[i]['area'] == 'Outside') {
          var tmp = rest_response[i]['timestamp'];
          if ($scope.selectedPeriod == 'Last 15'){
            var lastFive = tmp.substring(11, tmp.length-3);
          }
          
          if ($scope.selectedPeriod == 'Day' || $scope.selectedPeriod == 'Month') {
            var lastFive = tmp.substr(tmp.length - 8, 5);
          }  
          
          if ($scope.selectedPeriod == 'Year'){
            var lastFive = tmp.substring(0,7);
          }
          
          if ($scope.selectedPeriod == '6 Months'){
            var lastFive = tmp.substring(0,7);
          }    

          timestamps_out.push(lastFive);
          areas_out.push(rest_response[i]['area']);
          temperatures_out.push(rest_response[i]['temperature']);
          humidity_out.push(rest_response[i]['humidity']);
        }

        // temperature
        if (rest_response[i]['area'] == 'Temperature') {
          var tmp = rest_response[i]['timestamp'];
          if ($scope.selectedPeriod == 'Last 15'){
            var lastFive = tmp.substring(11, tmp.length-3);
          }
          
          if ($scope.selectedPeriod == 'Day' || $scope.selectedPeriod == 'Month') {
            var lastFive = tmp.substr(tmp.length - 8, 5);
          }  
          
          if ($scope.selectedPeriod == 'Year'){
            var lastFive = tmp.substring(0,7);
          }
          
          if ($scope.selectedPeriod == '6 Months'){
            var lastFive = tmp.substring(0,7);
          }    

          timestamps_temperature.push(lastFive);
          areas_temperature.push(rest_response[i]['area']);
          temperatures_temperature.push(rest_response[i]['temperature']);
        }
      }
      
      var data_left = {
        "xDataLeft": timestamps_left,
        "yDataLeft":[{
          "name": "Temperature [°C]",
          "data": temperatures_left
        },{
          "name": "Humidity [%]",
          "data": humidity_left
        }]
      };
      
      var data_right = {
        "xDataRight": timestamps_right,
        "yDataRight":[{
          "name": "Temperature [°C]",
          "data": temperatures_right
        },{
          "name": "Humidity [%]",
          "data": humidity_right
        }]
      };

      var data_out = {
        "xDataOut": timestamps_out,
        "yDataOut":[{
          "name": "Temperature [°C]",
          "data": temperatures_out
        },{
          "name": "Humidity [%]",
          "data": humidity_out
        }]
      };
      
      var data_temperature = {
        "xDataTemperature": timestamps_temperature,
        "yDataTemperature":[{
          "name": "Temperature [°C]",
          "data": temperatures_temperature
        }]
      };
      
      $scope.lineChartXDataLeft=data_left.xDataLeft;
      $scope.lineChartYDataLeft=data_left.yDataLeft;
      
      $scope.lineChartXDataRight=data_right.xDataRight;
      $scope.lineChartYDataRight=data_right.yDataRight;
      
      $scope.lineChartXDataOut=data_out.xDataOut;
      $scope.lineChartYDataOut=data_out.yDataOut;

      $scope.lineChartXDataTemperature=data_temperature.xDataTemperature;
      $scope.lineChartYDataTemperature=data_temperature.yDataTemperature;
    });
  }
  
  function getMisEle($scope, $http, period){
    var rest_response = {};
    // Electronic Measurements
    var timestamps = []; 
    var voltage = [];
    var current = [];
    var power = [];
    
  if (period == "Day") {
    if (isValid($scope.day)) {
      range = $scope.day;
    } else {
      range = moment().format('YYYY-MM-DD');
    }
  } else {
    range = $scope.selectedPeriod;
  }
    
    $http.get('ws/getMisElePeriod.php/?period=' + range).then(function(response){
      rest_response = response.data;
      for(var i=0; i < rest_response.length; i++){
        
      var tmp = rest_response[i]['timestamp'];
          if ($scope.selectedPeriod == 'Last 15'){
            var lastFive = tmp.substring(11, tmp.length-3);
          }
          
          if ($scope.selectedPeriod == 'Day' || $scope.selectedPeriod == 'Month') {
            var lastFive = tmp.substr(tmp.length - 8, 5);
          } 
          
          if ($scope.selectedPeriod == 'Year'){
            var lastFive = tmp.substring(0,7);
          }
          
          if ($scope.selectedPeriod == '6 Months'){
            var lastFive = tmp.substring(0,7);
          }
          
          timestamps.push(lastFive);
          voltage.push(rest_response[i]['voltage']);
          current.push(rest_response[i]['current']);
          power.push(rest_response[i]['power']);
      }
      
      var data = {
        "xData": timestamps,
        "yData":[{
          "name": "Voltage [V]",
          "data": voltage
        },{
          "name": "Current [A]",
          "data": current
    },{
          "name": "Power [W]",
          "data": power
    }]
      };
            
      $scope.lineChartXDataEle=data.xData;
      $scope.lineChartYDataEle=data.yData;
      
    });
  }
  
  function getMisMoisture($scope, $http, period){
    var rest_response = {};
    // Electronic Measurements
    var timestamps = []; 
    var moisture = [];
   
  if (period == "Day") {
    if (isValid($scope.day)) {
      range = $scope.day;
    } else {
      range = moment().format('YYYY-MM-DD');
    }
  } else {
    range = $scope.selectedPeriod;
  }
  
  $http.get('ws/getMisMoisturePeriod.php/?period=' + range).then(function(response){
      rest_response = response.data;
      for(var i=0; i < rest_response.length; i++){
        
      var tmp = rest_response[i]['timestamp'];
          if ($scope.selectedPeriod == 'Last 15'){
            var lastFive = tmp.substring(11, tmp.length-3);
          }
          
          if ($scope.selectedPeriod == 'Day' || $scope.selectedPeriod == 'Month') {
            var lastFive = tmp.substr(tmp.length - 8, 5);
          } 
          
          if ($scope.selectedPeriod == 'Year'){
            var lastFive = tmp.substring(0,7);
          }
          
          if ($scope.selectedPeriod == '6 Months'){
            var lastFive = tmp.substring(0,7);
          }

          timestamps.push(lastFive);
          moisture.push(rest_response[i]['moisture']);
      }
      
      var data = {
        "xData": timestamps,
        "yData":[{
          "name": "Moisture [?]",
          "data": moisture
        }]
      };
            
      $scope.lineChartXDataMoisture=data.xData;
      $scope.lineChartYDataMoisture=data.yData;
      
    });
  }
  
  function isValid(value) {
    // STRING FORMAT yyyy-mm-dd
    if(value=="" || value==null){return false;}                
    
    // m[1] is year 'YYYY' * m[2] is month 'MM' * m[3] is day 'DD'          
    var m = value.match(/(\d{4})-(\d{2})-(\d{2})/);
    
    // STR IS NOT FIT m IS NOT OBJECT
    if( m === null || typeof m !== 'object'){return false;}        
    
    // CHECK m TYPE
    if (typeof m !== 'object' && m !== null && m.size!==3){return false;}
          
    var ret = true; //RETURN VALUE            
    var thisYear = new Date().getFullYear(); //YEAR NOW
    var minYear = 1999; //MIN YEAR
    
    // YEAR CHECK
    if( (m[1].length < 4) || m[1] < minYear || m[1] > thisYear){ret = false;}
    // MONTH CHECK      
    if( (m[2].length < 2) || m[2] < 1 || m[2] > 12){ret = false;}
    // DAY CHECK
    if( (m[3].length < 2) || m[3] < 1 || m[3] > 31){ret = false;}
    
    return ret;    
  }
  
  function getPhoto($scope, $http){
    return $http.get('ws/getPhoto.php').then(function(result){
      $scope.src_photo = result.data['images'];
      $scope.src_photo_timestamp = result.data['timestamp'];
    });
  };

  function getPhotos($scope, $http){
    return $http.get('ws/getPhotos.php').then(function(result){
      var rest_response = result.data;
      var src_photos = [];
      for(var i=0; i < rest_response.length; i++){
           src_photos.push(rest_response[i].images);
      }
      $scope.src_photos = src_photos;
    });
  };

function getLight($scope, $http){
    var light;
    return $http.get('ws/getProbe.php?probe=light_left').then(function(result){
      if (result.data['status'] == 0) {
        $scope.src_light = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADIAAAAyCAYAAAAeP4ixAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAU0SURBVGhD7VlpiBxVEC4PlKgoovGHF+KNYIwHSIIXiqAkYma6x43RqIgH4i/jgmKU9a+KCCK6+icqHuCPRCR4gSYS1EQWd47d7CYiRpQsERSPJB7RrF89v2mqX89Opqd7DmQ+KOh5VfVV1bv6vR4ZYIABuofZETm4GsgltUCGq6G8XAvlEzyPOfnveQ3aV1VLcvGsyEF06x9M3yzHI8ERFPEtEp5tRWC/Q322LpPjSNM7TJTkMCS1Gj2920+0VUHxv2GEHlYu0nYXk0U5C0mMN0quHVEu5SR9dnwWyjwM+ZJKKOezKQH03mUYhZ8aJaTiplggL+L50UpJ7lDRZ/C+1Gz6QfcjbBYxTALjQ3IaeJdWV8ixbGqM7dfJ4fVeBuF+DPk9VEWYDORS6PfYBJx9IP8gyGuVglxE0zmhNuB/w/l4PJA9GoOmEcB9FXz+UBv4zdSG5BSqkpgI5WpLCsfNVDloj2ivWRvalTFKC2nWMpDwhUiw6vOh7QfwnUozB7S9bm0mAnmAqiTKQ3K27SU8v0KVvFWSQ9D2eV0XSSBrEfQomqWG+oLnbZ8XsTfpdk4zLeTxmE1RbqCqMWB0m44EiF61WyN64P4YkUog67VAmrQN5UC893x+5HEvTWRsqRyBeM+i7Qvd5dicDroBgHiXF2RirCTH0CQzlAuJTnsxdmpsmmQHSO+zAVR0PVGdG5D4tYk4gdxNdXaAMLY2EPB9qnIHRuVDL9anVGVDuSAngGy/Ja8EciPVuaNalGU2lm4+egSiun1gdwgsMXpsb67z1gMX9V4bEx1ZoLo1TC2XE9EDC+zJFL8fi5EG8hFVHQMS32BjorBHqBLNTU8dmiub4sCiuh1O++i4tl4MnvWoYQuJ3i2dgsawMZHDqLZrTpob2zXXlc7BAo1TNKgnvMC1e29U9NZTzqGD0Bg2pubg2pFTrB05OwcLGG80BvvqQwdSvSBFziB7wjl0EIjxpI2J3NZou+aE57+iXDAFnYMFdotzWYyOTDRkIH2m7uiEvdNJIOabXsynqdKZsxIyhSI+xlo5h80HBpzuigghINjRyWsqr8uxoz5+30l1+8Ab/DxL6gRHaqpzBzoq8XbX2UJ1NiDxbR75OqpyBwp5x4uVXNDtAuSrPHJ9u19PdW7Qo7kfB504THV2lG+VI0H4vRfgmzy/gHwZynw/BtbGzPZb5GiapMd4KCdp8vzpANKbbBAXKJQP8rqPoIjYYVEFu9IKmjhosXr548+54XaM+hwN5OdKUa6gygHt79pAKigw83tFORK82F5jR6VQlsDud+rX2dtjApj3l/tkVDm44Q/lO2uj4vdcGqivzwfZ5Z+lUMQma6NfcqhKolaQC6wxCkncOzBSi0H6p7VD2+52vklhaz9TfT2uv9Gh19AkAtrXWzs9OFLVGEh+xJHrlovC2BwDiGIvSRX4bd5wpRxKkwPC3dPh4/PM9XVE3+JqD9mJ3KLTcGZgVJ73k0gzxRpOKZ6pugr3MS+Usk0ExU02XYQEjyGTnu9WvVTRpLvQvwdQTOwa3HQREmpjfZSj0dfFrsJfhK3MX9itjvlgW6eqd0DiwzYpTJEt2HUeaiZqE/MJ5UHS9Q5zvAdSSZpNomMYFGKkLwrBfE8cJtOKcpCud8BiX9wouTTS861X4Y7goXztJ9eqYDS+yuMqkAuwfS6C/Noo0WaCIn7pi9GwqA3JGUjuBRSkfxLN+EnXRXXOJpTnqkU5ne79CU0QScf+GIqK6PfkfVQKcjI2glHINGVUr9BUD/A/h8i/uAaRGgMUbXQAAAAASUVORK5CYII=";
      } else {
        $scope.src_light = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADIAAAAyCAYAAAAeP4ixAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAATRSURBVGhD7ZlbqFRVGMdPF7qKPaQ+aERoguiLqSCJD1H00ENUmAY9hSgS4oN6QNMzs95kZq+19jR0KO1FBTPwoUTELpAiER4hSMQrISqJcgKFUoOgOv6/Pd9s1lp7zdkze++ZM8T84IM5893Xba89Z2jAgAG9QwjxsAjDZUKpYci+stInhdI/k9DnstR78f2WklJLhyYmHmK3/gHFzSgrJSDXUfREm3KNfLbv+uRZDjN1YAYeQzE7UdQ9p8i2pSzVXQzEdorFYXvLSLU6H0384isui1AsisnhewNGcCWS33GLaQotMeyHPULqEdh+EEnj8+ek8/mQYHZul4Lay5ymu5SkXI5i7ieKUPpfFHtAVMMlbNoSsoHtwcgnEUfdpxxs2h1ErfYCjZqbXEh1RgS1xWzWNiNKvQTfs554v4vKx8+zWbGsPnToETRxKpFUqa/E6Og0NusY8kWMw25cLM0f6Thns+LAlG90k0GOUoNskhkepG/c+GWtN7BJMWzW+kms53ErkVTntlUqz7BJbigWZuaSk+Mm5WaT/GA2PrQSREnkq6y2GAnDRdjIo76Zou9IRzb8lQWe+K+7ecqBXs/q/CT2htTfssqCCmzOHJr/wmwmWj74rqHT462aQezvzVzYKz+xKh8Y+VmY4v/M4KUgfIvVMY2ZwGlj2DWbMZuIBba+ZrC83jbtGke0msHq7JS0XmUGRgF/+dYtGn4Xun8sWwg1kGiChGzhw+4xQux5inKYtqVAv8Pq7KCIkhkUI/QDqxLA9j1fMwmBDdmyWwLkOGH7qB2syk501TCCYr/sZ5WX1GZSmiAoh+Ozm1XZQVJrWaAxyaqWiEC/b/pYAh2btYRymD5UA6uyg2ndZwWVusoqL3zEHjR9LIHOPM18IGdg+iDnXlZlB6NRs4JOMjqpTTQlpRnk+NK0RyOaVdnB6Kwzg0Ku+V5TWzYRnU6ePdOiGbpfoRHnqq/Wsjo7OCIX2kEhUr7C6hiM2takXWNjk/iaIR92j/E93dHIAlbnAwkvm4Hx99esisFIPmFd/JzTyW0msoUPq2NQ9JE4RmSnL7IqPwi+xQxOgpefN1gdEzfjNNGk2UyrJkoyfNPNg9zDrM7PVimfRgE3zAQo5qrvFxAqEMvjNf4zAel8TXxUr89M5FD6lqjXp7NJMQgZrjGTNBKp79KO0nbgg8K6LEbSxjMnEwh+zE2GNTzpc6UdKIYbF7NzvGs/4tH0Y7p/SyTNMXK+WwByjItqdTabdAdsvhVI9LeT/F6W36SE1i+SrxULh8Fke6xQ0Iz7kCQZwyZ+lE1SifYF+ThxylJuZpPegM35qVtEJ0vMu6SKuFN1yqZ6/XEsgzNWIUqfx6yk/oRDNpGt4YtYF+ilik16C9byUhRgvQZj2a1kdUvIxvJBjK7/upgGCjlqFdXG2xyeP/QLvuGjj7Fq6kDhw2ZReOKfxlrfNrmo07ZP8vLYc3ybtmPJ8RwqjEEjpvRFI57LZMeCGBxu6sBmX+EtrgOZ8qOXoKsGTqErvgLbEfj+WsSrQCHQ//1Q1J9ukWmCJv7oi9kw2RkE8/Cg+wwFjuH6ccstuimsG8NdbVSE4Vx270+oQBRs/2Oo2US/F+8iKpXncIfajcPgUiT4vEPrOawe8D9naOgBnYkRnMB9Tm8AAAAASUVORK5CYII=";
      }
    });
  };
  
  function getFanLamp($scope, $http){
    return $http.get('ws/getProbe.php?probe=fan_lamp').then(function(result){
      if (result.data['status'] == 0) {
        $scope.src_fan_lamp = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADIAAAAyCAYAAAAeP4ixAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAP3SURBVGhD7VnJThRRFL0OC3VrjBtjHOMcP8EfcCFUEdy5RPdshSAQQkgcdupaWKBEdOWQSMTElVFGDWEBaoJGfgBNBM8pbzVVz1fVr153A4l1kpvuvDuerlf1bt+SEiVKlPh/sdYp26eb5PxUIFenQ7kDGYa8VBnG+u1IB5s1kW3qtnUwGcq5qPBAvuNzzUlgS2ITTXJWw2weZkI5jWKeTIWyai3WQdR3dKpZTmrYjcPYBdkJAj2Qn2ZhvhLFCqSLsTVNYzF1WfYj8bhZSEoCWYAMorh+fLZT9PsgrsCi1Wddxt+Hsk/TNQYfWuUQipmzJI/3fDe2yBE1zwRtQKgXPj8yYs3NtMhBNa8v+Cuh0HkzabQlQrnxNpTdauoM+tBXY5hx5+t+ZfSeeGMmiySQu2rmDcawxg7l9XCL7FCz2gES3ZYkFcE26VTTwoBvhy1mRfAAUNPaMN0qpxDslzVJQnzIVCUBgc0KzqkT6uIPBHqaCoz9jLX7ybWKrgCZLBKMbd4zWBtRNz/w1EWQ1GHHbUYdi06uV/QOZGCTRSLyxWevsb7Kwzdy9gGCsF9aT4bH5cIV2aVqLzLQ5ZIg+DTDD7Zs6G+quhiiBtDoneKrkUQRMi4kYmDNvCpLXo0mO9RkIMpEKIdVnQILMW0pyQLx3ZkEMdssx01bNqeqdgccr6UCoe1QlRV5ZIqSiIEd8Nmwb1OVO+Bo3h8PVJWJLDI2qUaCgM1QyieQW6pyB4KMGEH6VZULFzIuJAjYDaT8AnmoKncgyPNkEFyRdlVVRR4ZVxIEcxq+z1TlDjolgxQkYr0nKJtB5FEqiPvWyiQRiysZ2NVha+H/dDKI481elUQsLmRgk77ZfQ5FEOEkJBlkUVVWZJFgwZQsnbpbgRrq8Pi1HIjohI+qOoU8EmpCm0JkbAei18SF7QC2k0uLUpVEjCJkkLvPsPFrUQgESB2KILJsNI3OJGK4kHl3UfYwl6H3axqJ2UDOIEBWG1+YRAza5PniM90wBvK7pjaeQKBRIyjnT/eSaxWdA4kYOWTq/8eK4ATQDGyTIiRiZJFJCmxWeOOrS23AFeiyJYnFh0SMamSg71DT2sGRDILap4sNHAeBxFhdx0FE3oAO0uM7oEOxvdatG8hcw0anHGMywT9JIShmGbq+rEMzCTyBjtE28rHEgnyabJIDat4Y8FdCotwhNgr8gl96CDKAgv8OsfGda9B/Ne2TAptXHy/JXk3XWHDfImGHdUt4CmNBrtf9nnABJ4Ao4jFI+b/owWEH/5G6PWJrQfTmCi0EivpmK9YmsF+iD0eyGmbrgE2dTifboiL5MjSQF5HwZSjJQkebLfkytESJEiU2CCJ/AGkHNa4gc5W9AAAAAElFTkSuQmCC";
      } else {
        $scope.src_fan_lamp = "data:image/png;base64,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";
      }
    });
  };
  
  function getFanIn($scope, $http){
    return $http.get('ws/getProbe.php?probe=fan_in').then(function(result){
      if (result.data['status'] == 0) {
$scope.src_fan_in = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADIAAAAyCAYAAAAeP4ixAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAP3SURBVGhD7VnJThRRFL0OC3VrjBtjHOMcP8EfcCFUEdy5RPdshSAQQkgcdupaWKBEdOWQSMTElVFGDWEBaoJGfgBNBM8pbzVVz1fVr153A4l1kpvuvDuerlf1bt+SEiVKlPh/sdYp26eb5PxUIFenQ7kDGYa8VBnG+u1IB5s1kW3qtnUwGcq5qPBAvuNzzUlgS2ITTXJWw2weZkI5jWKeTIWyai3WQdR3dKpZTmrYjcPYBdkJAj2Qn2ZhvhLFCqSLsTVNYzF1WfYj8bhZSEoCWYAMorh+fLZT9PsgrsCi1Wddxt+Hsk/TNQYfWuUQipmzJI/3fDe2yBE1zwRtQKgXPj8yYs3NtMhBNa8v+Cuh0HkzabQlQrnxNpTdauoM+tBXY5hx5+t+ZfSeeGMmiySQu2rmDcawxg7l9XCL7FCz2gES3ZYkFcE26VTTwoBvhy1mRfAAUNPaMN0qpxDslzVJQnzIVCUBgc0KzqkT6uIPBHqaCoz9jLX7ybWKrgCZLBKMbd4zWBtRNz/w1EWQ1GHHbUYdi06uV/QOZGCTRSLyxWevsb7Kwzdy9gGCsF9aT4bH5cIV2aVqLzLQ5ZIg+DTDD7Zs6G+quhiiBtDoneKrkUQRMi4kYmDNvCpLXo0mO9RkIMpEKIdVnQILMW0pyQLx3ZkEMdssx01bNqeqdgccr6UCoe1QlRV5ZIqSiIEd8Nmwb1OVO+Bo3h8PVJWJLDI2qUaCgM1QyieQW6pyB4KMGEH6VZULFzIuJAjYDaT8AnmoKncgyPNkEFyRdlVVRR4ZVxIEcxq+z1TlDjolgxQkYr0nKJtB5FEqiPvWyiQRiysZ2NVha+H/dDKI481elUQsLmRgk77ZfQ5FEOEkJBlkUVVWZJFgwZQsnbpbgRrq8Pi1HIjohI+qOoU8EmpCm0JkbAei18SF7QC2k0uLUpVEjCJkkLvPsPFrUQgESB2KILJsNI3OJGK4kHl3UfYwl6H3axqJ2UDOIEBWG1+YRAza5PniM90wBvK7pjaeQKBRIyjnT/eSaxWdA4kYOWTq/8eK4ATQDGyTIiRiZJFJCmxWeOOrS23AFeiyJYnFh0SMamSg71DT2sGRDILap4sNHAeBxFhdx0FE3oAO0uM7oEOxvdatG8hcw0anHGMywT9JIShmGbq+rEMzCTyBjtE28rHEgnyabJIDat4Y8FdCotwhNgr8gl96CDKAgv8OsfGda9B/Ne2TAptXHy/JXk3XWHDfImGHdUt4CmNBrtf9nnABJ4Ao4jFI+b/owWEH/5G6PWJrQfTmCi0EivpmK9YmsF+iD0eyGmbrgE2dTifboiL5MjSQF5HwZSjJQkebLfkytESJEiU2CCJ/AGkHNa4gc5W9AAAAAElFTkSuQmCC";
      } else {
        $scope.src_fan_in = "data:image/png;base64,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";
      }
    });
  };
  
  function getFanOut($scope, $http){
    return $http.get('ws/getProbe.php?probe=fan_out').then(function(result){
      if (result.data['status'] == 0) {
        $scope.src_fan_out = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADIAAAAyCAYAAAAeP4ixAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAP3SURBVGhD7VnJThRRFL0OC3VrjBtjHOMcP8EfcCFUEdy5RPdshSAQQkgcdupaWKBEdOWQSMTElVFGDWEBaoJGfgBNBM8pbzVVz1fVr153A4l1kpvuvDuerlf1bt+SEiVKlPh/sdYp26eb5PxUIFenQ7kDGYa8VBnG+u1IB5s1kW3qtnUwGcq5qPBAvuNzzUlgS2ITTXJWw2weZkI5jWKeTIWyai3WQdR3dKpZTmrYjcPYBdkJAj2Qn2ZhvhLFCqSLsTVNYzF1WfYj8bhZSEoCWYAMorh+fLZT9PsgrsCi1Wddxt+Hsk/TNQYfWuUQipmzJI/3fDe2yBE1zwRtQKgXPj8yYs3NtMhBNa8v+Cuh0HkzabQlQrnxNpTdauoM+tBXY5hx5+t+ZfSeeGMmiySQu2rmDcawxg7l9XCL7FCz2gES3ZYkFcE26VTTwoBvhy1mRfAAUNPaMN0qpxDslzVJQnzIVCUBgc0KzqkT6uIPBHqaCoz9jLX7ybWKrgCZLBKMbd4zWBtRNz/w1EWQ1GHHbUYdi06uV/QOZGCTRSLyxWevsb7Kwzdy9gGCsF9aT4bH5cIV2aVqLzLQ5ZIg+DTDD7Zs6G+quhiiBtDoneKrkUQRMi4kYmDNvCpLXo0mO9RkIMpEKIdVnQILMW0pyQLx3ZkEMdssx01bNqeqdgccr6UCoe1QlRV5ZIqSiIEd8Nmwb1OVO+Bo3h8PVJWJLDI2qUaCgM1QyieQW6pyB4KMGEH6VZULFzIuJAjYDaT8AnmoKncgyPNkEFyRdlVVRR4ZVxIEcxq+z1TlDjolgxQkYr0nKJtB5FEqiPvWyiQRiysZ2NVha+H/dDKI481elUQsLmRgk77ZfQ5FEOEkJBlkUVVWZJFgwZQsnbpbgRrq8Pi1HIjohI+qOoU8EmpCm0JkbAei18SF7QC2k0uLUpVEjCJkkLvPsPFrUQgESB2KILJsNI3OJGK4kHl3UfYwl6H3axqJ2UDOIEBWG1+YRAza5PniM90wBvK7pjaeQKBRIyjnT/eSaxWdA4kYOWTq/8eK4ATQDGyTIiRiZJFJCmxWeOOrS23AFeiyJYnFh0SMamSg71DT2sGRDILap4sNHAeBxFhdx0FE3oAO0uM7oEOxvdatG8hcw0anHGMywT9JIShmGbq+rEMzCTyBjtE28rHEgnyabJIDat4Y8FdCotwhNgr8gl96CDKAgv8OsfGda9B/Ne2TAptXHy/JXk3XWHDfImGHdUt4CmNBrtf9nnABJ4Ao4jFI+b/owWEH/5G6PWJrQfTmCi0EivpmK9YmsF+iD0eyGmbrgE2dTifboiL5MjSQF5HwZSjJQkebLfkytESJEiU2CCJ/AGkHNa4gc5W9AAAAAElFTkSuQmCC";
      } else {
        $scope.src_fan_out = "data:image/png;base64,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";
      }
    });
  };
  
  function getPump($scope, $http){
    return $http.get('ws/getProbe.php?probe=pump').then(function(result){
      if (result.data['status'] == 0) {
        $scope.src_pump = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADIAAAAyCAYAAAAeP4ixAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAALYSURBVGhD7ZnNaxNBGMYjHhQ/ToqgHkRtu7uJFKT0IigIov4RoojeFPHg/d18IGjFUq0gwZPeevIDkplEyT9QBMWzIngS1CL4rcTnDa+gzStNk5ndBPYHD02zO+/zzGRmN7PJZWRkpEtQbBwLyR6Vf0eTQrm2N4ztO9Y4NffI26PFFD3cEJF5HsW2LXrG78nh0SGMzb2/OtERpthdOTwahNQ4u7wTf4RpdkpOG24w6vsQ9pPWCRYfK5QakZw+nOyi1np05IXWgX+EtTM2V1snzYaPiOw1NbiikMxVaTZcoBOHEPDX8sD/FdmfUbF+UJoPB8GV+5sR7lVX2JVE5iW3lTLpg0DzatDedFPKpEuhWJvuTBM95MpC27Bkp6RcSiwsrMWifaoGXI3ILnItqZo86MRFNVgfQq3zUjZZJsluw0h+1EL1JTJLE9TaKuWTA+azXWEGFO76M1I+GcYrj3dGsfmihRlIZD4H1NghNv7BfL6tBnEic0ts/DJRtrvRke96iMGF6fWNPcTOH/hSeEML4FizYueHPLU2YcP0QTF2KvaYnLEbxdY9vCnSjL2I7AmxdQ8MHnUZehI+lQdi6xbeNGGUvmqmXgQv9hR7d6Aw7zd0U08KyBwQe3egIxc0M5/Cmjwn9u7AnC1rZl5FpiT27kDR66qZR2Hw5sTeHejIJc3Mp3DzrYi9O/i6rpl51hmxd0dUau5XjPwKnmLvkHZ7DabXG9XQg7A+XrOnuLuFv2Jrpl5EZl5s3ZOnWh5rpf8nJj0Kn8YP78+HsRe5o5k7Fdmq2Pmjs80l+1YN4EKoHVbq28XOL0HRHOadnBpkAHVqJv08uPNjjsv1glr52JyW8smSJ3McV5clNdgqhDv4e/71V8qmQ1CqB+iM0QL2IkynOgZkTMqlT0T1IwjW6mm68YPr2DzhtSbNh4/wcnMLLtEnEbbKYfF3kSWvq3yMz5HTMzIyEiGX+w3OYeh9QIYB0gAAAABJRU5ErkJggg==";
      } else {
        $scope.src_pump = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADIAAAAyCAYAAAAeP4ixAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAHMSURBVGhD7Za/SsNgFMXrn8VXcHTWNxAXrW/g4uDoqKCDg/hv6zPUWSh9BQvdOqTJIs6dCoprQYRK47lwhOulxcR+X+xwf3Cgycl3zk2aNqk5juM4juM4juMsCv1+fz9N04NFlszIcWeDg55wcL7Ikhk57mz8RCpU0RNpQSk0NItH3F+lRmaGIfe3OO7vZFl2rEOgZ1qVIZ16BpmJVnGSJNnTIdA7rcqQTj2DzESrOFi0oUNEuCLrtKMjXbZfZqJdnG63u4rFYxO2TTs60mW6xzIT7XJg8UCH4Ud2RCs60qW7oQGt8iCso8OwfUsrOtJluju0yoPFTRP2QCs60mW6m7TKg4BTHQa9ttvtFdrRkA4M/qa7sX1CuzxYvKXDGHhGOxroOLe9+MfapF2ePM+XEIqcH6Gf0HWv11vjYcGQTGTfsEN3JjILD/sbOJE6NDHB8s18QBk+P4aQZEkmPtueCZ4puxxnPhB2ZQuqErovOcb88Ba7QLB9QMbUWDrnvqWmgeD7KYVRJF2sDQuuzDLC7av9nbyRhpBkmeyhdLI+HCjb0UXQS8giyZJM3SGdtMOBK1RHeONb2D6kFQzJNB11Wo7jOI7jOM6/Uat9AQDqRo02Lc92AAAAAElFTkSuQmCC";
      }
    });
  };

  function getPeltier($scope, $http){
    return $http.get('ws/getProbe.php?probe=peltier').then(function(result){
      if (result.data['status'] == 0) {
        $scope.src_peltier = "data:image/svg+xml;base64,PD94bWwgdmVyc2lvbj0iMS4wIiA/PjxzdmcgaGVpZ2h0PSI0OCIgdmlld0JveD0iMCAwIDQ4IDQ4IiB3aWR0aD0iNDgiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyI+PHBhdGggZD0iTTAgMGg0OHY0OGgtNDh6IiBmaWxsPSJub25lIi8+PHBhdGggZD0iTTI0IDRjLTExLjA1IDAtMjAgOC45NS0yMCAyMHM4Ljk1IDIwIDIwIDIwIDIwLTguOTUgMjAtMjAtOC45NS0yMC0yMC0yMHptMCAzNmMtOC44NCAwLTE2LTcuMTYtMTYtMTYgMC0zLjcgMS4yNy03LjA5IDMuMzctOS44bDIyLjQzIDIyLjQzYy0yLjcxIDIuMS02LjEgMy4zNy05LjggMy4zN3ptMTIuNjMtNi4ybC0yMi40My0yMi40M2MyLjcxLTIuMSA2LjEtMy4zNyA5LjgtMy4zNyA4Ljg0IDAgMTYgNy4xNiAxNiAxNiAwIDMuNy0xLjI3IDcuMDktMy4zNyA5Ljh6Ii8+PC9zdmc+";
      } else {
        $scope.src_peltier = "data:image/png;base64,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";
      }
    });
  };

  function getWaterLevel($scope, $http){
    return $http.get('ws/getProbe.php?probe=water_level').then(function(result){
      if (result.data['status'] == 0) {
        $scope.src_water_level = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACwAAABACAYAAABhspUgAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAE3SURBVGhD7ZlBDYRADEWRgAQcgANwgASkgANwgBQkIAEJSJhtNxy6pMxAmG4g+S/5FzL8vsvMpYl7GRC2BsLWQNiaKMLLsriiKFySJGqqqnLrum6n7xFFuGkaVVSmbdvt9D2iCJdlqUrKQPgOjxOe59l1XXeYLMtUSRm+eNq/nGEYvhf3DEHhvu9VAYuM47hNPcYrzE+RVmyVNE2Dz59XeJomtdgyPNPHJWF+b/lbzOR5/jODv/m4JBzrpkv2LwyEYwNhWQZhAsKyDMIEhGUZhAkIyzIIExCWZRAmICzLIExAWJZBmICwLIMwAWFZBmECwrIMwgSEZdnrhHkfx/u2mOHNkZxxS5gXirLsH+GZPrzCzH7LYxmeFSIozCvVuq7VATHDM86sb4PCTwPC1kDYGghb8zJh5z7QNZmoilr2YQAAAABJRU5ErkJggg==";
      } 
      if (result.data['status'] == 20) {
        $scope.src_water_level = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACkAAAA+CAYAAAC2szLVAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAFFSURBVGhD7ZnhjUVAFIV1QAeUoAM6UAIdKIEOlEAHdEAHSlCCEmZzhWTW3pHdmTMvb/Pul5w/E879fpiREKh/gEiiEEkUnyHZdZ0KgoBNFEWq7/vzSnucJLdtY+XuccWpYZ5nVuoeV0TyiiuPDfu+q2EYVNu2bKqqYqXu4e69Mk3TOc2MUZIEkyRhh6KTpukxz4RRsq5rttBXmqY5J//EKJllGVvmKzTPxK8laZMgc38JQCTRkKjeL5K2iCQKkUQhkihEEoVIohBJFCKJQiRRiCQKkUQhkihEEoVXyXVd1bIs1qHv7IRXSX3NJkVRHD1vLXnJiKS+ZpOXSIZh+G39rynL8ujxKonicyXzPIeGfovo/VaSdJ7pJb5znZ8cRslxHNkyX6F5Jh4fNvqNEccxW4oK9dOcJ/A7wgMiiUIkUYgkBqW+AA3KUBKUNpd0AAAAAElFTkSuQmCC";
      } 
      if (result.data['status'] == 40) {
        $scope.src_water_level = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAC0AAAA8CAYAAADykDOkAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAFaSURBVGhD7ZnhrYMgFIUdwRHcQDfQDRzBUXQD3cBRdANHcARH4OU0klACBQuS+nK+5KQJWu73Q64GMvFAKJ0KSqcimvS+76KqKpFlmTFN04jjOM67w4gm3XWdUVZN3/fn3WFEk67r2iiqhtIx+EnpbdvEMAzWFEVhFFWDxWj6LzJN02sx++AlPY6jUeKOzPN8VrXjlEabMk1+V/I8d7ZGp/SyLMbJ7wxqfuKyNPoxxmKmLMu3Ghj7xGXpWB1ARe88lEYofUJpQGkLlAaUtkBpQGkLlAaUtkBpQGkLlAb/ShqbKur41WB/Q+4oJZPGrzr+TaQcpQGltSSXxiYhroVEklR6XdegSB71eGCjHvCZBpTWklw6xjlM8td4TCgNdGmcF+I8MGbwxafWCJZGL1UnTBHZv204pYF++nRnUMuFlzRWedu2xiIxgxo+R81e0r8GpVNB6VQ8UFqIP0KwwBysSQFcAAAAAElFTkSuQmCC";
      } 
      if (result.data['status'] == 60) {
        $scope.src_water_level = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADQAAAA8CAYAAAApK5mGAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAGDSURBVGhD7ZqBqYMwFEUdwRHcQDfQDRzBUXQD3cARHEE3cISO4Aj53GLg/ZDSRJP/g9wDF0tqeTlQnxrN1MOgUOpQKHUo5MLr9VJVVaksy6xpmkYdx3HuHZYoQl3XWUVk+r4/9w5LFKG6rq0SMhRyhEJg33c1DMPHFEVhlZBBY7D9Fpmm6d1YruAtNI6jdYIxMs/zWdUdLyG0WlvhWMnz3Lu9ewmt62otHDOo6cMtIZxvMBYyZVn+qoExH24JxehUZoek0Ll1gkIXoJABhWQxCjlAIQMKyWIUcoBCBhSSxbTQt1tyl2iSEMJWjl+JnjiFDCgkiz1WaFmW90TuRK/u4LOs8S9CIUlCCAuCctw3WH/TK6VJCD3uGKKQJRT6QBAhtFx8dyeaZIS2bbsVzWP+crhiBzyGDCgkiz1WKMRz16QufUJCIYNbQngfAe8bhAyuvGWNqEI4V8hifxF9fnLFSwiYT6ljBrV88RZCN2rb1jqBkEGNK6/HeAulDoVSh0Jpo9QPIzpEJuhEo/YAAAAASUVORK5CYII=";
      } 
      if (result.data['status'] == 80) {
        $scope.src_water_level = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACwAAAA8CAYAAAAdUliaAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAGISURBVGhD7ZlhzYMwEIaRgIQ5AAfgAAlIAQfgAAlIAAdIQAIS+u1daHLfUiis12bN7knebOm2u+dH21FIVGSIsG9E2Dcswuu6qjzPVZIkxpRlqbZt27/tBotwXddGUZqmafZvu8EiXBSFUZJGhF34OuFlWVTbtod5PB5GSRosPNNvkb7vXwv3ClbhruuMAj4yDMPe9ZhTYWxFpsK+kqapdfs7FZ6myVjYZ9DzjFvC2G8xxpksy/71wNgZt4S5VjrlfYcRYW5EmBYT4SciTItR4XmePw6uTTRBhPFKxz+Jlo5GWIuJMC0WrTDHNXLQOcxJEGHbkelKNDKHaTERfiLCtJgWHsfx1cgl+nSM97SHF2FOggjjhgcdvxvcf9B3eoIIRzeHRdgwfsRvCGNLwmcu0QQTNp3V7kQTzZSQMx0QYZKgwhxnuqB/zZyIMC2G53F43sYZXLnRHk7C2CtpsRDR+/MRp8Lg/SmPz6CXDaswVnNVVcYGnEGPK49vrcLfhgj7RoR9E5mwUn8uqsW8S6m81gAAAABJRU5ErkJggg==";
      } 
      if (result.data['status'] >= 100) {
        $scope.src_water_level = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADQAAAA7CAYAAAA0Lqk+AAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAGfSURBVGhD7ZmBrYIwEIbZQDfAEdhANoANdANG0A1gA9gANtANGIERHKEvP5GkNveStrTPe3Bf8icGtdcvetaWRG0MEeKOCNlS17VKkoTM8XhUbdu+XxmWKELTNJEiZmIQZdTH40EKmImBCNnw74Rer5fquk7d73cy1+uVFDBDvXfJMAzvam44C0HmdDqREwydLMvmei44C1VVRRaPldvt9q5sh7PQ+XwmC8cK6rmwWgg/ACFjLsh/LhQaSOnjf02oLEuV57l38MkANkL6NZ8sExchAxHajRCaGougbyAC2AhhD/R8Pr2z/MXZzFeuKIp5HOkhAxHajdDhcPi47hp2PRQKNkJN05BbadtABEgPGYiQCFmGnRB+dvGcb9jtWEPBRmjt4SPO+4D0kIEIiZBl2AmN4zhPxjfYwgM81sf/qhB1VmAbdkL6NZ/ImcIviNBuhNaeKVwul3kcNkKhECGD1ULU3bg1wa18ffzoQlgv9IKxs6xPtjgL9X1PFo4V1HPBqwmwXU7TlJxAqGD8ZVvuQviu/jIixB0R4s7GhJT6AaGgaaCB1MW2AAAAAElFTkSuQmCC";
      } 
    });
  };
  
  function wavelengthToColor(wavelength) {
        var r,
            g,
            b,
            alpha,
            colorSpace,
            wl = wavelength,
            gamma = 1;
  
        if (wl >= 380 && wl < 440) {
            R = -1 * (wl - 440) / (440 - 380);
            G = 0;
            B = 1;
       } else if (wl >= 440 && wl < 490) {
           R = 0;
           G = (wl - 440) / (490 - 440);
           B = 1;  
        } else if (wl >= 490 && wl < 510) {
            R = 0;
            G = 1;
            B = -1 * (wl - 510) / (510 - 490);
        } else if (wl >= 510 && wl < 580) {
            R = (wl - 510) / (580 - 510);
            G = 1;
            B = 0;
        } else if (wl >= 580 && wl < 645) {
            R = 1;
            G = -1 * (wl - 645) / (645 - 580);
            B = 0.0;
        } else if (wl >= 645 && wl <= 780) {
            R = 1;
            G = 0;
            B = 0;
        } else {
            R = 0;
            G = 0;
            B = 0;
        }
 
        // intensty is lower at the edges of the visible spectrum.
        if (wl > 780 || wl < 380) {
            alpha = 0;
        } else if (wl > 700) {
            alpha = (780 - wl) / (780 - 700);
        } else if (wl < 420) {
            alpha = (wl - 380) / (420 - 380);
        } else {
            alpha = 1;
        }
 
        colorSpace = ["rgba(" + (R * 100) + "%," + (G * 100) + "%," + (B * 100) + "%, " + alpha + ")", R, G, B, alpha]
 
        // colorSpace is an array with 5 elements.
        // The first element is the complete code as a string.  
        // Use colorSpace[0] as is to display the desired color.  
        // use the last four elements alone or together to access each of the individual r, g, b and a channels.  
       
        return colorSpace;
       
    }
  
  function getLightSpectrum($scope, $http){
    return $http.get('ws/getProbe.php?probe=light_spectrum').then(function(result){
    var light_spectrum = result.data['status'];
    $scope.src_light_spectrum = light_spectrum;
    $scope.src_light_spectrum_r = wavelengthToColor(light_spectrum)[1];
    $scope.src_light_spectrum_g = wavelengthToColor(light_spectrum)[2];
    $scope.src_light_spectrum_b = wavelengthToColor(light_spectrum)[3];
    });
  };
});
