import React, { Fragment, useState, useEffect } from "react";
import ReactDOM from "react-dom";
import Editor from "rich-markdown-editor";

export function NotesViewer() {

  const [notes, setNotes] = useState("Notes go here");

  return <Editor
  defaultValue={notes}
/>
}