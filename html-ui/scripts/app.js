
(function(document) {
    'use strict';

    var app = document.querySelector('#app');

    /* defaults */
    app.qtConnected = false;


    // Listen for template bound event to know when bindings
    // have resolved and content has been stamped to the page
    app.addEventListener('dom-change', function() {
        /* jshint ignore:start */
        // TODO: this may not be the correct place to put our once-app-is-ready stuff
        if (typeof qt !== 'undefined' && typeof qt.webChannelTransport !== 'undefined') {
            app.qtConnected = true;
            document.getElementById('visingoMainToolbar').qtConnected = true;
       
            new QWebChannel(qt.webChannelTransport, function(channel) { // jshint ignore:line

                app.webchannel = channel;
                app.visingocontrol = channel.objects.visingocontrol;

                console.log('creating qwebchannel object');
                console.log(channel);

                app.visingocontrol.visualizerLoaded.connect(function(visName) {
                    console.log('signal: loaded visualizer ' + visName);
                    console.log(app.visingocontrol.loadedVisualizers);
                    console.log(JSON.parse(app.visingocontrol.loadedVisualizers));
                });

                app.visingocontrol.sendTestJson.connect(function(blob) {
                    console.log('signal: sendTestJson');
                    console.log(blob);
                });

                app.visingocontrol.sendTestJsonStr.connect(function(blob) {
                    console.log('signal: sendTestJsonStr');
                    console.log(blob);
                    console.log(JSON.parse(blob));
                });




                /*
                // Connect to a signal:
                channel.objects.foo.mySignal.connect(function() {
                    // This callback will be invoked whenever the signal is emitted on the C++/QML side.
                    console.log(arguments);
                });

                // To make the object known globally, assign it to the window object, i.e.:
                window.foo = channel.objects.foo;

                // Invoke a method:
                foo.myMethod(arg1, arg2, function(returnValue) {
                    // This callback will be invoked when myMethod has a return value. Keep in mind that
                    // the communication is asynchronous, hence the need for this callback.
                    console.log(returnValue);
                });

                // Read a property value, which is cached on the client side:
                console.log(foo.myProperty);

                // Writing a property will instantly update the client side cache.
                // The remote end will be notified about the change asynchronously
                foo.myProperty = "Hello World!";

                // To get notified about remote property changes,
                // simply connect to the corresponding notify signal:
                foo.onMyPropertyChanged.connect(function(newValue) {
                    console.log(newValue);
                });

                // One can also access enums that are marked with Q_ENUM:
                console.log(foo.MyEnum.MyEnumerator);
                */
            });

        } // end if qt defined

        /* jshint ignore:end */
    });

    // See https://github.com/Polymer/polymer/issues/1381
    window.addEventListener('WebComponentsReady', function() {
        // imports are loaded and elements have been registered
    });
    /* jshint ignore:end */

})(document);

