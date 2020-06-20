import React, { Fragment, useState, useEffect } from "react";
import ReactDOM from "react-dom";
import css from "./style.css";
import { FixedSizeList as List } from 'react-window';
// var React = require('react');
var AsciiTable = require('./AsciiViewer');
var AsciiSet = AsciiTable.AsciiSet;
var PixelSet = require('./PixelTable').PixelSet;
var PixelSet2 = require('./PixelTable').PixelSet2;
var createReactClass = require('create-react-class');

function Item(props) {
	var classes = (props.active ? 'active' : '') + (props.value == -1 ? ' none' : '');
	return (<li title={(props.row_start_address+props.index).toString(16)} className={classes} onMouseOver={()=>props.activate(props.index)} onMouseLeave={()=>props.clear()}>{props.byteString}</li>);
}

var Item2 = createReactClass({
	activate: function () {
		this.props.activate(this.props.index);
	},
	clear: function() {
		this.props.clear();
	},
	render: function() {
		var classes = (this.props.active ? 'active' : '') + (this.props.value == -1 ? ' none' : '');
		return (<li title={this.props.row_start_address+this.props.index} className={classes} onMouseOver={this.activate} onMouseLeave={this.clear}>{this.props.byteString}</li>);
	}
});

var Set = createReactClass({
	activate: function () {
		this.props.activateSet(this.props.index);
	},
	clear: function() {
		this.props.clearSet();
	},
	render: function() {
		var items = this.props.set.map(function(b, i) {
			var byteString = "";

			if (b != -1 ) {
				byteString = b.toString(16);

				if(byteString.length == 1) {
					byteString = "0" + byteString;
				}
			}

			var active = this.props.activeItem == i && this.props.active;
			return (<Item row_start_address={this.props.row_start_address} index={i} value={b} byteString={byteString} active={active} activate={this.props.activateItem} clear={this.props.clearItem}/>);
		}.bind(this));

		return (
			<ul className={"setHex" + (this.props.active ? ' active' : '')} onMouseOver={this.activate} onMouseLeave={this.clear}>
				{items}
			</ul>
		);
	}
});

// 
// # For Highlighting - Each row has both an active set and an active item
// The active set is one of the 4 sets of 4 bytes and the active itsem is which of those 4 bytes was selected.
// 
var Row = createReactClass({
	getInitialState: function() {
		return ({
			activeSet: -1,
			activeItem: -1
		});
	},
	setActiveSet: function (activeSet) {
		if(this.props.sets[activeSet][this.state.activeItem] == -1) return;
		this.setState({activeSet: activeSet});
	},
	clearActiveSet: function () {
		this.setState({activeSet: -1});
	},
	setActiveItem: function (activeItem) {
		this.setState({activeItem: activeItem});
	},
	clearActiveItem: function () {
		this.setState({activeItem: -1});
	},
	activate: function () {
		this.setActiveSet(this.props.index);
	},
	render: function() {
		var sets = this.props.sets.map(function(set, i) {
			var active = this.state.activeSet == i ? true : false;

			var props = {
				set: set,
				index: i,
				active: active,
				activeItem:   this.state.activeItem,

				activateSet:  this.setActiveSet,
				clearSet:     this.clearActiveSet,
				activateItem: this.setActiveItem,
				clearItem:    this.clearActiveItem,
				row_start_address: this.props.row_start_address
			};

			return (<Set {...props}/>);
		}.bind(this));


		var asciiSets = this.props.sets.map(function(set, i) {
			var active = this.state.activeSet == i ? true : false;

			var props = {
				set: set,
				setIndex: i,
				index: i,
				active: active,
				activeItem:   this.state.activeItem,
				activeSet: this.state.activeSet,
				activateSet:  this.setActiveSet,
				clearSet:     this.clearActiveSet,
				activateItem: this.setActiveItem,
				clearItem:    this.clearActiveItem
			};

			return (<AsciiSet {...props}/>);
		}.bind(this));

		var pixelSets = this.props.sets.map(function(set, i) {
			var active = this.state.activeSet == i ? true : false;

			var props = {
				set: set,
				setIndex: i,
				index: i,
				active: active,
				activeItem:   this.state.activeItem,
				activeSet: this.state.activeSet,
				activateSet:  this.setActiveSet,
				clearSet:     this.clearActiveSet,
				activateItem: this.setActiveItem,
				clearItem:    this.clearActiveItem
			};

			return (<PixelSet {...props}/>);
		}.bind(this));

		return (
			<div className="row">
				<div className="heading">{this.props.heading}:</div>
        <div className="hex-sets">
          {sets}
        </div>
				<div className="ascii">
					{asciiSets}
				</div>
				<div className="pixels">
					{pixelSets}
				</div>
			</div>
		);
	}
});

function Hex(props) {
	const { bytesper, rows, offset=0, offsetDisplay="hex" } = props;
  const pad = "000000";


		const createRow = ({ index, style }) => {
			const row = rows[index];
			// Heading is the offset index shown on the left
			var row_start_address = index * bytesper + offset;
			var heading = ""+row_start_address;
			if (offsetDisplay === "hex") {
				heading = (row_start_address).toString(16);
			}
			heading = pad.substring(0, pad.length - heading.length) + heading;
			return <div style={style}><Row sets={row} heading={heading} row_start_address={row_start_address}/></div>
		};

		let height = 350;
		if (rows.length * 25 < height) {
			height = rows.length * 25;
		}

		return (
			<div className="hexviewer">
				<div className="hex">
				<List
					height={height}
					itemCount={rows.length}
					itemSize={25}
					width={'100%'}
				>
					{createRow}
				</List>
				</div>
			</div>
		);
}

const HexViewer = function(props) {
	const { rowLength, setLength, original_buffer: buffer, offset } = props;
		var rowChunk = rowLength, setChunk = setLength;
		var rows = [], row = [], set = [], sets = [];
		
		var temp_buffer = [];
		var bytes = props.buffer.length;

		if(Buffer.isBuffer(props.buffer)) {
			for (var ii = 0; ii < bytes; ii++) {
				temp_buffer.push(props.buffer[ii]);
			}
		} else {
			temp_buffer = props.buffer;
		}

		for (var i = 0; i<bytes; i+=rowChunk) {
			sets = [];
			let temparray = temp_buffer.slice(i,i+rowChunk);

			for(var z = temparray.length; z < rowChunk; z++) {
				temparray.push(-1);
			}
			row = [];
			for (let x=0,k=temparray.length; x<k; x+=setChunk) {
				set = temparray.slice(x,x+setChunk);

				for(z = set.length; z < setChunk; z++) {
					set.push(-1);
				}
				row.push(set);

			}
			rows.push(row);
		}

		return (
			<Hex rows={rows} bytesper={rowChunk} offset={offset} />
		);
	};

export default HexViewer;
