
(function(document) {
    'use strict';

    var app = document.querySelector('#app');

    /* Defaults for bindings */
    app.visualizationType = 'playback';
    app.solverLogFile = [];
    app.visualizers = [];
    app.inputFiles = []; // NOTE: array
    app.parseStatus = 'outdated';

    // disconnected mode
    app.setDcModeVisualizers = function(evt) {
        app.visualizers = evt.target.lastResponse.visualizers;
        app.chosenVisualizer = app.visualizers[0];
    };

    // Listen for template bound event to know when bindings
    // have resolved and content has been stamped to the page
    app.addEventListener('dom-change', function() {
        // disconnected mode
        if(!app.qtConnected) {
            app.$.ajaxDisconnectedModeEnabledVisualizers.generateRequest();
        }
    });

    // See https://github.com/Polymer/polymer/issues/1381
    window.addEventListener('WebComponentsReady', function() {
        // imports are loaded and elements have been registered
    });

    // interactive mode
    /* jshint ignore:start */
    app.initMapping = function(visualizer) {
        var visPredicates = document.querySelector('#vis-predicates');
        console.log(visualizer);
        app.visingocontrol.loadVisualizer(visualizer, function(ret) {
            console.log('loadVisualizer return val: ' + ret);
            if(ret === true) {
                console.log('loaded visualizers: ');
                console.log(app.visingocontrol.loadedVisualizers);
                console.log(JSON.parse(app.visingocontrol.loadedVisualizers));
            }
        });
        // try to find if visualizer exists
        // load its define.json
        //    2 above by same command on webchannel, returns define.json or false
        // put define.json obj somewhere for now
        // populate visPredicates
    };
    /* jshint ignore:end */

})(document);
