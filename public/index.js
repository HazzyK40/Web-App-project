

	jQuery(document).ready(function() {

	//Init load js
	init_all();
	function init_all() {
		collapse_panel();
		$("fileHolder").val("");
		
	}


	function collapse_panel() {
		var coll = document.getElementsByClassName("collapsible");
		var i;

			for (i = 0; i < coll.length; i++) {
			  coll[i].addEventListener("click", function() {
			    this.classList.toggle("active");
			    var content = this.nextElementSibling;
			    if (content.style.maxHeight){
			      content.style.maxHeight = null;
			    } else {
			      content.style.maxHeight = content.scrollHeight + "px";
			    } 
			  });
			}
	}

	function insert_file_log(filename){
		var rowCount = document.getElementById('file-tab').childElementCount;
		 //console.log("row count : " + rowCount);
		    if(rowCount >= 2)
		    {
		    	document.getElementById("file-tab-scroll").style.overflow = "auto";
		    }
		var htmlStr = '<td><a href="#">'+filename+'</a></td>'+
						'<td>2.5</td>'+
						'<td>Alonzo Church</td>'+
						'<td>657</td>'+
						'<td>75</td>'+
						'<td>545</td>'+
						'</tr>';
		var tableRef = document.getElementById('file-tab').getElementsByTagName('tbody')[0];
		var newRow = tableRef.insertRow(tableRef.rows.length);
		newRow.innerHTML = htmlStr;
	}

    // On page-load AJAX Example
    jQuery.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/stub-check-uploads',   //The server endpoint we are connecting to
        
        success: function (data) {
            //console.log("stub to list uploaded files");
            var htmlStr = '<div id="file-tab-scroll" class="table-wrapper-scroll my-custom-scrollbar">'+
						'<table id="file-tab"  class="table table-hover">'+
						'<thead>'+
						'<tr>'+
						'<th>File name  (click to download)</th>'+
						'<th>Version</th>'+
						'<th>Creator</th>'+
						'<th>Number of waypoints</th>'+
						'<th>Number of routes</th>'+
						'<th>Number of tracks</th>'+
						'</tr>'+'</thead>'+'<tbody>';
			var valDropDown = "";
			$.each(data, function (i, jsonRec) {
				valDropDown += '<option value="'+jsonRec.filename+'">'+jsonRec.filename+'</option>';
					
				var innerHTMLStr ='<tr>'+
						'<td><a href="#">'+jsonRec.filename+'</a></td>'+
						'<td>'+jsonRec.version+'</td>'+
						'<td>'+jsonRec.creator+'</td>'+
						'<td>'+jsonRec.num_waypoints+'</td>'+
						'<td>'+jsonRec.num_routes+'</td>'+
						'<td>'+jsonRec.num_tracks+'</td>'+
						'</tr>';
			  	htmlStr += innerHTMLStr;
			});
			var endHTMLStr= '</tbody>'+
						    '</table>'+
						    '</div>';
			htmlStr += endHTMLStr;
            jQuery('#file-log-t').html(htmlStr);

            //GPX view file
            var gpxHtmlStr = valDropDown;
  			jQuery('#gpx-div-dd').html(gpxHtmlStr);
            // To observe/alter the generated html
    		$('#file-tab').on('change', 'input', function () {
		    var rowCount = document.getElementById('file-tab').childElementCount;
		    console.log("row count : " + rowCount);
		    if(rowCount > 3)
		    {
		    	document.getElementById("file-tab-scroll").style.overflowY = "auto";
		    }
	});
          

        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#file-log-t').html("On page load, received error from server");
            console.log(error); 
        }
    });

    // Event listener form example , we can use this instead explicitly listening for events
    // No redirects if possible
    /*
    $('#uploadForm').submit(function(e){
        alert("I am an alert box!");
        e.preventDefault();
       
    });
	*/
	$("#fileHolder").change(function () {
        var fileExtension = ['gpx'];
        if ($.inArray($(this).val().split('.').pop().toLowerCase(), fileExtension) == -1) {
            alert("Wrong file format. Upload only .gpx files");
            $(this).val('');
        }
    });

	$("#uploadForm").submit(function(e) {
	    e.preventDefault(); 
	    var form = $(this);
	    var filepath = $("#fileHolder").val();
	    var filename = "";
	    //console.log(filepath);
	    if(filepath.indexOf("/") != -1)
	    {
               filename = filepath.split("/").pop();
	    }
	    else if(filepath.indexOf("\\") != -1)
	    {
               filename = filepath.split("\\").pop();
	    }
	   	var formData = new FormData($(this)[0]);
	   	//console.log(formData);
	    var url = form.attr('action'); 
	    if(filepath)
	    {
		    
		    $.ajax({
		         type: "POST",
		         url: '/upload',
		         data: formData,
		         processData: false,
    			 contentType: false, 
		         success: function(data){
		             alert("Uploaded " + filename);
		             console.log("Uploaded " + filename);
		             insert_file_log(filename);
		         },
	        fail: function(error) {
	            
	            console.log(error); 
	        }

		    });
		}
		else
		{
			alert("Please select a .gxp file first");
		}
		return false;
	});

	$("#openGpx").submit(function(e) {
	    e.preventDefault(); 
	    var form = $(this);
	   	var formData = new FormData($(this)[0]);
	   	//console.log(formData);
	    var url = form.attr('action'); 
	   
		    $.ajax({
		         type: "post",
		         url: '/open-gpx',
		         data: formData,
		         processData: false,
    			 contentType: false, 
		         success: function(data){
		        
		             console.log("opening gpx file ");
		              var htmlStr = '<div id="file-tab-scroll" class="table-wrapper-scroll my-custom-scrollbar">'+
						'<table id="file-tab"  class="table table-hover">'+
						'<thead>'+
						'<tr>'+
						'<th>Component</th>'+
						'<th>Name</th>'+
						'<th>Number of points</th>'+
						'<th>Length</th>'+
						'<th>Loop</th>'+
						'</tr>'+'</thead>'+'<tbody>';
						var valDropDown = "";
						$.each(data, function (i, jsonRec) {
							valDropDown += '<option value="'+jsonRec.filename+'">'+jsonRec.filename+'</option>';
								
							var innerHTMLStr ='<tr>'+
									'<td><a href="#">'+jsonRec.component+'</a></td>'+
									'<td>'+jsonRec.name+'</td>'+
									'<td>'+jsonRec.num_pts+'</td>'+
									'<td>'+jsonRec.length+'</td>'+
									'<td>'+jsonRec.loop+'</td>'+
									'</tr>';
						  	htmlStr += innerHTMLStr;
						});
						var endHTMLStr= '</tbody>'+
									    '</table>'+
									    '</div>';
						htmlStr += endHTMLStr;
			            jQuery('#gpx-view-t').html(htmlStr);
		            
		         },
	        fail: function(error) {
	            
	            console.log(error); 
	        }

		    });
		
		
	});
   
 


    

	
});