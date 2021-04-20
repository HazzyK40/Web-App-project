
var express = require('express');
var app = express();
var path = require('path');

const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// const port = process.argv[2];
const port = 3000;

app.use(express.static(__dirname + '/public'));

//route hanlding


//route hanlding
app.get('/stub-check-uploads', function(req, res) {
	dummy_flog = [
	  {
	    "filename": "temp.gxp",
	    "version": 2,
	    "creator": "Bill Joy",
	    "num_waypoints": 39,
	    "num_routes": 29,
	    "num_tracks": 4653
	  },
	  {
	    "filename": "file2.gxp",
	    "version": 12.2,
	    "creator": "Donald Knuth",
	    "num_waypoints": 339,
	    "num_routes": 67,
	    "num_tracks": 43653
	  }
	  
	]

    res.json(dummy_flog);
});

app.post('/open-gpx', function(req, res) {
	var filename = req.body.gpxfiles; 
	var dummy_flog = [];
	//console.log(filename);
    if(filename==='temp.gpx')
    {
    	dummy_flog=[{
    		"component" : "Route 1",
    		"name": "different route",
    		"num_pts": 23,
    		"length": "192m",
    		"loop": "FALSE"
    	},
    	{
    		"component" : "Track 1",
    		"name": "same route",
    		"num_pts": 234,
    		"length": "152m",
    		"loop": "FALSE"
    	}
    	]
    }
    else if (filename==='file2.gpx')
    {
    	dummy_flog=[{
    		"component" : "Route 12",
    		"name": "prev route",
    		"num_pts": 84,
    		"length": "922m",
    		"loop": "FALSE"
    	},
    	{
    		"component" : "Track 16",
    		"name": "re route",
    		"num_pts": 87,
    		"length": "812m",
    		"loop": "FALSE"
    	}
    	]
    }
    else{
    	dummy_flog=[{
    		"component" : "Route 74",
    		"name": "detour route",
    		"num_pts": 41,
    		"length": "28m",
    		"loop": "FALSE"
    	},
    	{
    		"component" : "Track 71",
    		"name": "new route",
    		"num_pts": 14,
    		"length": "75m",
    		"loop": "FALSE"
    	}
    	]
    }
    res.json(dummy_flog);
});
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
 
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }
    res.end('{"success" : "file uploaded", "status" : 200}');
    //res.redirect('/');
  });
});

app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: '+err);
      res.send('');
    }
  });
});

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});


app.listen(port);
console.log('Running app at localhost: ' + port);