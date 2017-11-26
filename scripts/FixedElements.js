//var FIXED_ELEMENTS = [];
var loaded = false;

function getXPathForElement(el, xml) {
    var xpath = '';
    var pos, tempitem2;

    while(el !== xml.documentElement) {
        pos = 0;
        tempitem2 = el;
        while(tempitem2) {
            if (tempitem2.nodeType === 1 && tempitem2.nodeName === el.nodeName) { // If it is ELEMENT_NODE of the same name
                pos += 1;
            }
            tempitem2 = tempitem2.previousSibling;
        }

        var prop = '';

        if(el.hasAttribute('id') && el.id != '') {
            prop = prop + el.id + '/';
        }

        if(el.hasAttribute('class') && el.className != '') {
            prop = prop + el.className + '/';
        }

        xpath = el.nodeName + '/'+ xpath + prop;

        el = el.parentNode;
    }
    xpath = xml.documentElement.nodeName+'/'+xpath;
    xpath = xpath.replace(/\/$/, '');
    return xpath;
}

function isElementInViewport(rect) {
    // if as high and as wide of the screen, ignore it
    var width = window.innerWidth;
    var height = window.innerHeight;
    return (
                (rect.bottom >= 0 && rect.right >= 0 &&
                rect.top <= height && rect.left <= width &&
                rect.height > 0 && (rect.height < height || rect.width < width))
                ||
                (rect.top >= 0 && rect.left >= 0 &&
                 rect.bottom <= height && rect.right <= width &&
                 rect.height > 0 && (rect.height < height || rect.width < width))
           );
}

function parseNumber(bounding) {
    return parseFloat(Math.round(bounding * 100) / 100).toFixed(2);
}

function getFixedElements(timeout) {
    var fxdElmnts = "";

    [].slice.call(document.querySelectorAll("*")).forEach(function(el,i){
        var style = window.getComputedStyle(el, null);
        if(style['position'] === "fixed" && style['display'] !== 'none' && style['visibility'] !== 'hidden' && style['opacity'] > 0.4) {
            var boundings = el.getBoundingClientRect();

            if(isElementInViewport(boundings)) {
                fxdElmnts += parseNumber(boundings['top']) + "," + parseNumber(boundings['right']) + "," + parseNumber(boundings['bottom']) +
                        "," + parseNumber(boundings['left']) + "," + parseNumber(boundings['width']) + "," + parseNumber(boundings['height']) +
                        "," + getXPathForElement(el, document) + "|";
            }
        }
    });

    if(fxdElmnts.length) {
        if(timeout) {
            console.log("timeout_fixedelements " + fxdElmnts);
            return fxdElmnts;
        } else { // window.onload
            console.log("demobrowser_fixedelements " + fxdElmnts);
        }
    }
}

function logMove(e) {
    console.log("demobrowser_move " + e.screenX + " " + e.screenY);
}

document.addEventListener('mousemove', logMove, true);

window.onload = function() {
    loaded = true;
    getFixedElements(0);
}
