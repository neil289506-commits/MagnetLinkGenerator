
let T='btih';
const TR=['udp://tracker.opentrackr.org:1337/announce','udp://open.tracker.cl:1337/announce','udp://tracker.torrent.eu.org:451/announce','udp://open.stealth.si:80/announce','udp://exodus.desync.com:6969/announce'];
document.querySelectorAll('.tab').forEach(b=>b.onclick=()=>{T=b.dataset.t;document.querySelectorAll('.tab').forEach(x=>x.classList.toggle('on',x===b));run()});
lt.onclick=()=>{const cur=tr.value.split('\n').map(s=>s.trim()).filter(Boolean);TR.forEach(t=>{if(!cur.includes(t))cur.push(t)});tr.value=cur.join('\n');run()};
function run(){
 const h=hs.value.trim(),w=[];
 if(!h){mo.textContent='請輸入 info hash';mw.textContent='';return}
 if(T==='btih'&&!(/^[0-9a-fA-F]{40}$/.test(h)||/^[A-Za-z2-7]{32}$/.test(h)))w.push('BTIH 應為 40 碼十六進位或 32 碼 Base32');
 if(T==='btmh'&&!(/^[0-9a-fA-F]+$/.test(h)&&h.length%2===0&&h.length>=4))w.push('BTMH 應為偶數長度的十六進位(至少 4 碼,含前綴)');
 let u='magnet:?xt=urn:'+T+':'+h;
 if(dn.value.trim())u+='&dn='+encodeURIComponent(dn.value.trim());
 if(xl.value.trim()){if(/^[1-9]\d*$/.test(xl.value.trim()))u+='&xl='+xl.value.trim();else w.push('檔案大小 xl 必須是正整數,已略過此欄位')}
 if(kt.value.trim())u+='&kt='+kt.value.split(',').map(s=>s.trim()).filter(Boolean).map(encodeURIComponent).join('+');
 tr.value.split('\n').map(s=>s.trim()).filter(Boolean).forEach(t=>u+='&tr='+encodeURIComponent(t));
 mo.textContent=u;mw.textContent=w.length?'⚠ '+w.join(';'):'✔ 格式正常';mw.className=w.length?'bad':'ok';
}
gen.onclick=run;[hs,dn,xl,kt,tr].forEach(e=>e.addEventListener('input',run));
cp.onclick=()=>{navigator.clipboard?.writeText(mo.textContent).then(()=>toast('已複製到剪貼簿'),()=>toast('請手動選取複製'))};
run();
