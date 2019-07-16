function rr=shiftc(vector)

for ii=1:(size(vector,2)-1)
vector(:,end-ii+1)=vector(:,end-ii);
end
rr=vector;
return