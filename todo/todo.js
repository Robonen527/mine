$(document).ready(function() {
    var ourListHTML = localStorage.getItem('ourListHTML');
    if (ourListHTML) {
        $('#ourList').html(ourListHTML);
    }

    $('#add').click(function (e) { 
        e.preventDefault();
        if ($("#tasks").val().length > 0) {
            $('#ourList').append("<li class=\"list-group-item tod\" aria-current=\"true\"><span calss=\"center\">" +
            $("#tasks").val() +
            "<div class=\"btn-group float-end\" role=\"group\" aria-label=\"Basic mixed styles example\">"+
            "<button type=\"button\" class=\"btn btn-danger up possition-relative\">up</button>"+
            "<button type=\"button\" class=\"btn btn-warning down possition-relative\">down</button>"+
            "<button type=\"button\" class=\"btn btn-success delete possition-relative\">delete</button></div>");  
            $("#tasks").val("");
            localStorage.setItem('ourListHTML', $('#ourList').html());
        }
    });
    $('#tasks').keydown(function(event) {
        if (event.keyCode === 13) {
            if ($("#tasks").val().length > 0) {
                $('#ourList').append("<li class=\"list-group-item tod\" aria-current=\"true\"><span calss=\"center\">" +
                $("#tasks").val() +
                "<div class=\"btn-group float-end\" role=\"group\" aria-label=\"Basic mixed styles example\">"+
                "<button type=\"button\" class=\"btn btn-danger up possition-relative\">up</button>"+
                "<button type=\"button\" class=\"btn btn-warning down possition-relative\">down</button>"+
                "<button type=\"button\" class=\"btn btn-success delete possition-relative\">delete</button></div>");  
                $("#tasks").val("");
                localStorage.setItem('ourListHTML', $('#ourList').html());
            }
        }
    });
    $("#ourList").on('click', ".delete", function() {
        $(this).parent().parent().parent().remove();
        localStorage.setItem('ourListHTML', $('#ourList').html());
    })
    $("#ourList").on('click', ".up", function() {
        var a = $(this).parent().parent().parent();
        a.insertBefore(a.prev());
        localStorage.setItem('ourListHTML', $('#ourList').html());
    })
    $("#ourList").on('click', ".down", function() {
        var a = $(this).parent().parent().parent();
        a.insertAfter(a.next());
        localStorage.setItem('ourListHTML', $('#ourList').html());
    })
    $("#ourList").on('mouseover', ".tod", function() {
        $(this).css("background-color","yellow");
    })
    $("#ourList").on('mouseout', ".tod", function() {
        $(this).css("background-color","white");
    })
  });