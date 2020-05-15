var React = require('react');
var createReactClass = require('create-react-class');


var AsciiCharacter = createReactClass({
	activate: function () {
		this.props.activateItem(this.props.itemIndex);
		this.props.activateSet(this.props.setIndex);
	},
	render: function() {
		return (<li index={this.props.currentCell} className={this.props.classes} onMouseOver={this.activate} onMouseLeave={this.props.clearSet}>{this.props.character}</li>); 

	}
});

function createAsciiCharacterElement (setIndex, byteValue, itemIndex, theSet) {
	var characterToDisplay = "·";
	if (byteValue > 31 && byteValue < 127) {
		characterToDisplay = String.fromCharCode(byteValue);
	}

	if (byteValue == -1) {
		characterToDisplay = "";
	}

	var activeCell  = this.props.activeSet * (theSet.length) + this.props.activeItem;
	var currentCell = setIndex*(theSet.length) + itemIndex;
	var classes = ( activeCell == currentCell ) ? 'active' : '';

	var props = {
		setIndex: setIndex,
		itemIndex: itemIndex,
		currentCell: currentCell,
		classes: classes,
		character: characterToDisplay,
		activateSet:  this.props.activateSet,
		clearSet:     this.props.clearSet,
		activateItem: this.props.activateItem
	};

	return (<AsciiCharacter {...props}/>);				
}

export const AsciiSet = createReactClass({
	activate: function () {
		this.props.activateSet(this.props.index);
	},
	render: function() {
		var items = this.props.set.map(createAsciiCharacterElement.bind(this, this.props.setIndex));

		return (
			<ul className={"setAscii" + (this.props.active ? ' active' : '')} onMouseOver={this.activate} onMouseLeave={this.props.clearSet}>
				{items}
			</ul>
		);
	}
});