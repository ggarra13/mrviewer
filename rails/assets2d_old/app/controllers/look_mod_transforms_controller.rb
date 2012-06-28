class LookModTransformsController < ApplicationController
  def index
    list
    render :action => 'list'
  end

  # GETs should be safe (see http://www.w3.org/2001/tag/doc/whenToUseGet.html)
  verify :method => :post, :only => [ :destroy, :create, :update ],
         :redirect_to => { :action => :list }

  def list
    @look_mod_transform_pages, @look_mod_transforms = paginate :look_mod_transforms, :per_page => 10
  end

  def show
    @look_mod_transform = LookModTransform.find(params[:id])
  end

  def new
    @look_mod_transform = LookModTransform.new
  end

  def create
    @look_mod_transform = LookModTransform.new(params[:look_mod_transform])
    if @look_mod_transform.save
      flash[:notice] = 'LookModTransform was successfully created.'
      redirect_to :action => 'list'
    else
      render :action => 'new'
    end
  end

  def edit
    @look_mod_transform = LookModTransform.find(params[:id])
  end

  def update
    @look_mod_transform = LookModTransform.find(params[:id])
    if @look_mod_transform.update_attributes(params[:look_mod_transform])
      flash[:notice] = 'LookModTransform was successfully updated.'
      redirect_to :action => 'show', :id => @look_mod_transform
    else
      render :action => 'edit'
    end
  end

  def destroy
    LookModTransform.find(params[:id]).destroy
    redirect_to :action => 'list'
  end
end
