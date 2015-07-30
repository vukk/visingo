/*
 *
 */
var testVisGraphVisualizer = (function (vis, _, parser) {
	var my = {},
		privateVariable = 1,
        context,
        container,
        canvas,
        entities,
        currentEntityKeys = {nodes: [], edges: []},
        attributes,
        network;

	function privateMethod() {
		// ...
	};


    my.initialize = function(visJsContainer) {
        container = visJsContainer;

        // TODO: remove development stuff
        var devNodes = [{id: 1, label: 'Node 1'},
        {id: 2, label: 'Node 2'},
        {id: 3, label: 'Node 3'},
        {id: 4, label: 'Node 4'},
        {id: 5, label: 'Node 5'}];
        var devEdges = [
        {from: 1, to: 3},
        {from: 1, to: 2},
        {from: 2, to: 4},
        {from: 2, to: 5}
        ];
        currentEntityKeys.nodes = [1,2,3,4,5];
        currentEntityKeys.edges = ['1%3', '1%2', '2%4', '2%5'];

        // entities are visjs network "data"
        entities = {
            //nodes: new vis.DataSet([]),
            nodes: new vis.DataSet(devNodes, {queue: true}),
            //edges: new vis.DataSet([]),
            edges: new vis.DataSet(devEdges, {queue: true})
        };

        // attributes are visjs network "options"
        attributes = {
            autoResize: true
        };
        //attributes = new vis.DataSet({});

        network = new vis.Network(container, entities, attributes);
        my.network = network; // TODO remove
        
        canvas    = container.firstChild.firstChild;
        context   = canvas.getContext('2d');
    };

    my.addOptionsAnswerSet = function(set) {
        var res = doParsing(set);
        if(res === false) return false;

        _.assign(attributes, res);
        delete attributes.nodes;
        delete attributes.edges;
    };

    function doParsing(set) {
        var toParse;
        if(_.isArray(set)) {
            toParse = set.join(' ');
        }
        else if(_.isString(set)) {
            toParse = set;
        }
        else 
            return false;

        return parser.parse(toParse);
    };

    my.addAnswerSet = function(set) {
        var res = doParsing(set);
        if(res === false) return false;

        var nodesToRem = _.difference(currentEntityKeys.nodes, Object.keys(res.nodes));
        var edgesToRem = _.difference(currentEntityKeys.edges, Object.keys(res.edges));
        var nodesToAdd = _.difference(Object.keys(res.nodes), currentEntityKeys.nodes);
        var edgesToAdd = _.difference(Object.keys(res.edges), currentEntityKeys.edges);

        entities.edges.remove(edgesToRem);
        entities.nodes.remove(nodesToRem);

        _.map(nodesToAdd, function(key) {
            entities.nodes.add(res.nodes[key]);
        });
        _.map(edgesToAdd, function(key) {
            entities.edges.add(res.edges[key]);
        });

        currentEntityKeys.nodes = 
            _.union(nodesToAdd, _.difference(currentEntityKeys.nodes, nodesToRem));
        currentEntityKeys.edges = 
            _.union(edgesToAdd, _.difference(currentEntityKeys.edges, edgesToRem));

        return true;
    };


	my.moduleProperty = 1;
	my.moduleMethod = function () {
		// ...
	};

	return my;
}(vis, _, visingo.parsers.answersettermsToVisjs));

