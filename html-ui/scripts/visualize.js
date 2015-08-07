
(function(document) {
    'use strict';

    var app = document.querySelector('#app');

    /* Defaults for bindings */

    // Listen for template bound event to know when bindings
    // have resolved and content has been stamped to the page
    app.addEventListener('dom-change', function() {
        //if(!app.qtConnected) {
        //    app.$.ajaxDisconnectedModeEnabledVisualizers.generateRequest();
        //}
    });

    // See https://github.com/Polymer/polymer/issues/1381
    window.addEventListener('WebComponentsReady', function() {
        // routes are also in WebComponentsReady, Firefox seems to care about this
        Polymer.Base.async(function() {
            // imports are loaded and elements have been registered
            //console.log('current params:', app.params);
            var stored = JSON.parse(window.localStorage[app.params.visualizer]);
            // TODO: just load all to app? by Object.keys(stored) app.key = ...
            app.visualizer = app.params.visualizer;
            app.visualizationType = stored.visualizationType;
            app.settings = stored.settings;
            app.inputFiles = stored.inputFiles;
        },1);
    });

})(document);
