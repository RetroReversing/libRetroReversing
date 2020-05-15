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
	var characterToDisplay = "Â·";
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

var colour_map = {
	0:"rgb("+255+","+255+","+255+")", //white
	1:"rgb("+192+","+192+","+192+")", //light grey
	2:"rgb("+96+","+96+","+96+")", // dark grey
	3:"rgb("+0+","+0+","+0+")" // black
};

var cachedPixels = {
};

//  2bpp Planar
function draw_row_from_bytes(byte1, byte2) {
	if (cachedPixels[byte1] && cachedPixels[byte1][byte2])
		return cachedPixels[byte1][byte2];
	var pixels = [];
	//
	// # Loop over each bit in the 2 bytes
	//
	for(var x = 8; x >0; x--)
	  {
		  // Find bit index for this pixel
		  // i.e get a mask which can be applied to a byte, that will return the value of this pixel
		  var pixelMask = 1 << (7-x);
		  var colourOfPixel = 0;
  
		  var bitValueOfByte1 = byte1 & pixelMask;
		  var bitValueOfByte2 = byte2 & pixelMask;
  
		  if (bitValueOfByte1) colourOfPixel+=1;
		  if (bitValueOfByte2) colourOfPixel+=2;
  
		  var fillStyle = colour_map[colourOfPixel];
		  let style = {'background-color': fillStyle};
		  pixels.push(<div className="pixel" style={style}></div>);
	  }

	  if (!cachedPixels[byte1]) cachedPixels[byte1] = {};
	  cachedPixels[byte1][byte2] = pixels;
	  return pixels;
	
	}

export const PixelSet = createReactClass({
	activate: function () {
		this.props.activateSet(this.props.index);
	},
	render: function() {
		var items = this.props.set.map(createAsciiCharacterElement.bind(this, this.props.setIndex));
		var pixels = [];
		var pixels1 = draw_row_from_bytes(this.props.set[0],this.props.set[1]);
		var pixels2 = pixels.concat(draw_row_from_bytes(this.props.set[2],this.props.set[3]));

		return (
			<div className={"pixelset" + (this.props.active ? ' active' : '')} onMouseOver={this.activate} onMouseLeave={this.props.clearSet}>
				<div className="codec2bpp">
					<div className="pixelRow">
					{pixels1}
					</div>
					<div className="pixelRow">
					{pixels2}
					</div>
				</div>
			</div>
		);
	}
});
