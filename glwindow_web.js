(pyodide) => {
  const canvas = document.createElement('canvas');
  canvas.width = 1280;
  canvas.height = 720;

  canvas.style.position = 'fixed';
  canvas.style.left = '50%';
  canvas.style.top = '50%';
  canvas.style.transform = 'translate(-50%, -50%)';

  const options = {
    powerPreference: 'high-performance',
    premultipliedAlpha: false,
    antialias: false,
    alpha: false,
    depth: false,
    stencil: false,
  };

  const gl = canvas.getContext('webgl2', options);
  document.body.appendChild(canvas);

  const setupRender = (update) => {
    const render = () => {
      update();
      requestAnimationFrame(render);
    };

    requestAnimationFrame(render);
  };

  return { gl, width: canvas.width, height: canvas.height, setupRender };
}
