export function downloadData(data, filename) {
  const blob = new Blob([data], {type: 'application/octet-stream'});
  console.error("Bobl", blob);

  const url = window.URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.style.display = 'none';
  a.href = url;
  // the filename you want
  a.download = filename;
  document.body.appendChild(a);
  a.click();
  window.URL.revokeObjectURL(url);
}